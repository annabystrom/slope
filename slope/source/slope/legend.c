/*
 * Copyright (C) 2017,2023  Elvis Teixeira, Anatoliy Sokolov
 *
 * This source code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <slope/item_p.h>
#include <slope/legend.h>
#include <slope/scale.h>

typedef struct _SlopeLegendPrivate
{
  GtkOrientation      orientation;
  double              user_x, user_y;
  double              entry_height;
  double              rect_stroke_width;
  SlopeColor          rect_stroke_color;
  SlopeColor          rect_fill_color;
  SlopeColor          text_color;
  gboolean            rect_antialias;
  graphene_rect_t     rect;
  SlopeCorner         anchor;
  GList *             items;
  guint               num_items;
  guint               num_visible_items;
  SlopeLegendPosition position;
} SlopeLegendPrivate;

G_DEFINE_TYPE_WITH_CODE (SlopeLegend, slope_legend, SLOPE_ITEM_TYPE, G_ADD_PRIVATE (SlopeLegend))

#define LEGEND_THUMB_WIDTH 40.0
#define LEGEND_PADDING 10.0

static void _legend_finalize(GObject *self);
static void _legend_draw(SlopeItem *self, cairo_t *cr);
static void _legend_get_figure_rect (SlopeItem *self, graphene_rect_t *rect);
static void _legend_get_data_rect (SlopeItem *self, graphene_rect_t *rect);
static void _legend_draw_rect(SlopeItem *self, cairo_t *cr);
static void _legend_draw_thumbs(SlopeItem *self, cairo_t *cr);
static void _legend_evaluate_rect(SlopeItem *self, cairo_t *cr);
static void _legend_evaluate_extents(SlopeItem *self, cairo_t *cr);

static void slope_legend_class_init(SlopeLegendClass *klass)
{
  GObjectClass *  object_klass = G_OBJECT_CLASS(klass);
  SlopeItemClass *item_klass   = SLOPE_ITEM_CLASS(klass);
  object_klass->finalize       = _legend_finalize;
  object_klass->finalize       = _legend_finalize;
  item_klass->draw             = _legend_draw;
  item_klass->get_data_rect    = _legend_get_data_rect;
  item_klass->get_figure_rect  = _legend_get_figure_rect;
}

static void slope_legend_init(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->orientation        = GTK_ORIENTATION_HORIZONTAL;
  priv->rect_fill_color    = SLOPE_WHITE;
  priv->rect_stroke_color  = SLOPE_BLACK;
  priv->text_color         = SLOPE_BLACK;
  priv->rect_stroke_width  = 1.0;
  priv->rect_antialias     = FALSE;
  priv->items              = NULL;
  priv->entry_height       = 0.0;
  priv->anchor             = SLOPE_TOPLEFT;
  priv->num_items          = 0;
  priv->num_visible_items  = 0;
  priv->position           = SLOPE_LEGEND_BOTTOM;
}

static void _legend_finalize(GObject *self)
{
  slope_legend_clear_items(SLOPE_LEGEND(self));
  G_OBJECT_CLASS(slope_legend_parent_class)->finalize(self);
}

SlopeItem *
slope_legend_new (GtkOrientation orientation)
{
  SlopeItem *self = SLOPE_ITEM(g_object_new(SLOPE_LEGEND_TYPE, NULL));
  slope_legend_set_orientation(SLOPE_LEGEND(self), orientation);
  return self;
}

static void _legend_draw(SlopeItem *self, cairo_t *cr)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  _legend_evaluate_extents(self, cr);
  if (priv->num_visible_items < 1)
    {
      return;
    }
  _legend_evaluate_rect(self, cr);
  _legend_draw_rect(self, cr);
  _legend_draw_thumbs(self, cr);
}

static void _legend_evaluate_extents(SlopeItem *self, cairo_t *cr)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  priv->rect.size = GRAPHENE_SIZE_INIT_ZERO;
  priv->entry_height       = 0.0;
  priv->num_visible_items  = 0;
  GList *item_iter         = priv->items;
  while (item_iter != NULL)
    {
      SlopeItem *item = SLOPE_ITEM(item_iter->data);
      if (slope_item_get_is_visible(item))
        {
          priv->num_visible_items += 1;
          const char *         item_name = slope_item_get_name(item);
          cairo_text_extents_t txt_ext;
          cairo_text_extents(cr, item_name, &txt_ext);
          if (txt_ext.height > priv->entry_height)
            {
              priv->entry_height = txt_ext.height;
            }
          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              priv->rect.size.width +=
                  (txt_ext.width + LEGEND_THUMB_WIDTH + 2.0 * LEGEND_PADDING);
            }
          else
            {
              if (txt_ext.width > priv->rect.size.width)
                {
                  priv->rect.size.width = txt_ext.width;
                }
            }
        }
      item_iter = item_iter->next;
    }
}

static void _legend_evaluate_rect(SlopeItem *self, cairo_t *cr)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  SlopeScale *        scale = slope_item_get_scale(self);
  /* top left point (in figure coordinates) is easy because
     it is given by the user. as the x and y coordinates
     in set_position */
  graphene_point_t user_pos = GRAPHENE_POINT_INIT (priv->user_x, priv->user_y);
  graphene_point_t pos = user_pos;
  if (scale != NULL)
    {
      /* if there is a scale, use it to transform the point.
         if there's none, use the user provided position as
         the legend's figure position */
      slope_scale_map(scale, &pos, &user_pos);
    }
  priv->rect.origin = pos;
  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      priv->rect.size.width += LEGEND_PADDING;
      priv->rect.size.height = priv->num_visible_items + 3.0 * LEGEND_PADDING;
    }
  else
    {
      priv->rect.size.width += LEGEND_THUMB_WIDTH + 3.0 * LEGEND_PADDING;
      priv->rect.size.height =
          priv->num_visible_items * (priv->entry_height + LEGEND_PADDING) +
          LEGEND_PADDING;
   }
}

static void _legend_draw_rect(SlopeItem *self, cairo_t *cr)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  cairo_set_line_width(cr, priv->rect_stroke_width);
  slope_cairo_set_antialias(cr, priv->rect_antialias);
  cairo_new_path(cr);
  slope_cairo_rect (cr, &priv->rect);
  slope_cairo_draw(cr, priv->rect_stroke_color, priv->rect_fill_color);
}

static void _legend_draw_thumbs(SlopeItem *self, cairo_t *cr)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  GList *             item_iter = priv->items;
  graphene_point_t pos = priv->rect.origin;
  pos.x = pos.x + LEGEND_PADDING + LEGEND_THUMB_WIDTH / 2.0;
  pos.y = pos.y + LEGEND_PADDING + priv->entry_height / 2.0;
  while (item_iter != NULL)
    {
      SlopeItem *item = SLOPE_ITEM(item_iter->data);
      if (slope_item_get_is_visible(item))
        {
          const char *item_name = slope_item_get_name(item);
          _item_draw_thumb(item, cr, &pos);
          slope_cairo_set_color(cr, priv->text_color);
          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              slope_cairo_text(
                  cr,
                  pos.x + LEGEND_THUMB_WIDTH / 2.0 + LEGEND_PADDING,
                  pos.y + priv->entry_height / 2.0,
                  item_name);
              cairo_text_extents_t txt_ext;
              cairo_text_extents(cr, item_name, &txt_ext);
              pos.x +=
                  (txt_ext.width + LEGEND_THUMB_WIDTH + 2.0 * LEGEND_PADDING);
            }
          else
            {
              slope_cairo_text(
                  cr,
                  pos.x + LEGEND_THUMB_WIDTH / 2.0 + LEGEND_PADDING,
                  pos.y + priv->entry_height / 2.0,
                  item_name);
              pos.y += priv->entry_height + LEGEND_PADDING;
            }
          cairo_stroke(cr);
        }
      item_iter = item_iter->next;
    }
}

static void
_legend_get_figure_rect (SlopeItem *self, graphene_rect_t *rect)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  graphene_rect_init_from_rect  (rect, &priv->rect);
}

static void
_legend_get_data_rect (SlopeItem *self, graphene_rect_t *rect)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (SLOPE_LEGEND (self));
  SlopeScale *        scale = slope_item_get_scale(self);
  if (scale == NULL)
    {
      graphene_rect_init (rect, 0.0, 0.0, 0.0, 0.0);
    }
  else
    {
      graphene_point_t data_pos;
      slope_scale_unmap(scale, &data_pos, &priv->rect.origin);
      graphene_rect_init (rect, data_pos.x, data_pos.y,
                          priv->user_x - data_pos.x, priv->user_y - data_pos.y);
    }
}

void
slope_legend_set_orientation (SlopeLegend *    self,
                              GtkOrientation orientation)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->orientation = orientation;
}

GtkOrientation
slope_legend_get_orientation (SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  return priv->orientation;
}

void slope_legend_set_anchor(SlopeLegend *self, SlopeCorner anchor)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->anchor = anchor;
}

SlopeCorner slope_legend_get_anchor(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  return priv->anchor;
}

void slope_legend_set_position(SlopeLegend *self, double x, double y)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->position           = SLOPE_LEGEND_CUSTOM;
  priv->user_x             = x;
  priv->user_y             = y;
}

void slope_legend_get_position(SlopeLegend *self, double *x, double *y)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  if (priv->position == SLOPE_LEGEND_CUSTOM)
    {
      *x = priv->user_x;
      *y = priv->user_y;
    }
}

void slope_legend_set_default_position(SlopeLegend *       self,
                                       SlopeLegendPosition position)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->position = position;
}

void slope_legend_add_item(SlopeLegend *self, SlopeItem *item)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->items              = g_list_append(priv->items, (gpointer) item);
}

void slope_legend_clear_items(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  if (priv->items != NULL)
    {
      g_list_free(priv->items);
      priv->items = NULL;
    }
}

void slope_legend_set_fill_color(SlopeLegend *self, SlopeColor color)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->rect_fill_color = color;
}

SlopeColor slope_legend_get_fill_color(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  return priv->rect_fill_color;
}

void slope_legend_set_stroke_color(SlopeLegend *self, SlopeColor color)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->rect_stroke_color = color;
}

SlopeColor slope_legend_get_stroke_color(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  return priv->rect_stroke_color;
}

void slope_legend_set_stroke_width(SlopeLegend *self, double width)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  priv->rect_stroke_width = width;
}

double slope_legend_get_stroke_width(SlopeLegend *self)
{
  SlopeLegendPrivate *priv = slope_legend_get_instance_private (self);
  return priv->rect_stroke_width;
}

/* slope/legend.c */
