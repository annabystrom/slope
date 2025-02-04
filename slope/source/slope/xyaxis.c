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

#include <slope/scale.h>
#include <slope/xyaxis.h>

typedef struct _SlopeXyAxisPrivate
{
  GtkOrientation orientation;
  guint32       component;
  double        min;
  double        max;
  double        anchor;
  GdkRGBA       line_color;
  GdkRGBA       grid_color;
  GdkRGBA       text_color;
  GdkRGBA       title_color;
  GdkRGBA       select_rect_color;
  gboolean      line_antialias;
  double        line_width;
  double        grid_line_width;
  char *        title;
  gboolean      selected;
  SlopeSampler *sampler;
} SlopeXyAxisPrivate;

G_DEFINE_TYPE_WITH_CODE (SlopeXyAxis, slope_xyaxis, SLOPE_ITEM_TYPE, G_ADD_PRIVATE (SlopeXyAxis))

static void _xyaxis_finalize(GObject *self);
static void _xyaxis_get_figure_rect (SlopeItem *self, graphene_rect_t *rect);
static void _xyaxis_get_data_rect (SlopeItem *self, graphene_rect_t *rect);
static void _xyaxis_draw(SlopeItem *self, cairo_t *cr);
static void _xyaxis_draw_horizontal(SlopeXyAxis *self, cairo_t *cr);
static void _xyaxis_draw_vertical(SlopeXyAxis *self, cairo_t *cr);

static void slope_xyaxis_class_init(SlopeXyAxisClass *klass)
{
  GObjectClass *  object_klass = G_OBJECT_CLASS(klass);
  SlopeItemClass *item_klass   = SLOPE_ITEM_CLASS(klass);
  object_klass->finalize       = _xyaxis_finalize;
  item_klass->draw             = _xyaxis_draw;
  item_klass->get_data_rect    = _xyaxis_get_data_rect;
  item_klass->get_figure_rect  = _xyaxis_get_figure_rect;
}

static void slope_xyaxis_init(SlopeXyAxis *self)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->orientation        = GTK_ORIENTATION_HORIZONTAL;
  gdk_rgba_parse (&priv->line_color, "black");
  gdk_rgba_parse (&priv->grid_color, "#78787840");
  priv->line_antialias     = FALSE;
  gdk_rgba_parse (&priv->text_color, "black");
  gdk_rgba_parse (&priv->select_rect_color, "0000FF64");
  gdk_rgba_parse (&priv->title_color, "black");
  priv->line_width      = 1.0;
  priv->grid_line_width = 1.0;
  priv->title           = NULL;
  priv->selected        = FALSE;
  priv->component =
      SLOPE_XYAXIS_LINE | SLOPE_XYAXIS_TICKS_DOWN | SLOPE_XYAXIS_TITLE;
  priv->sampler = slope_sampler_new();
}

static void _xyaxis_finalize(GObject *self)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (SLOPE_XYAXIS (self));
  slope_sampler_destroy(priv->sampler);
  G_OBJECT_CLASS(slope_xyaxis_parent_class)->finalize(self);
}

SlopeItem *
slope_xyaxis_new (GtkOrientation orientation, const char *title)
{
  SlopeXyAxis *self = SLOPE_XYAXIS(g_object_new(SLOPE_XYAXIS_TYPE, NULL));
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (SLOPE_XYAXIS (self));
  priv->orientation        = orientation;
  slope_xyaxis_set_title(self, title);
  return SLOPE_ITEM(self);
}

static void _xyaxis_draw(SlopeItem *self, cairo_t *cr)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (SLOPE_XYAXIS (self));
  cairo_set_line_width(cr, priv->line_width);
  slope_cairo_set_antialias(cr, priv->line_antialias);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      _xyaxis_draw_horizontal(SLOPE_XYAXIS(self), cr);
    }
  else if (priv->orientation == GTK_ORIENTATION_VERTICAL)
    {
      _xyaxis_draw_vertical(SLOPE_XYAXIS(self), cr);
    }
  if (priv->selected == TRUE)
    {
      graphene_rect_t rect;
      slope_item_get_figure_rect(self, &rect);
      gdk_cairo_set_source_rgba (cr, &priv->select_rect_color);
      slope_cairo_rect (cr, &rect);
      cairo_fill(cr);
    }
}

static void _xyaxis_draw_horizontal(SlopeXyAxis *self, cairo_t *cr)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  SlopeScale *         scale = slope_item_get_scale(SLOPE_ITEM(self));
  cairo_text_extents_t txt_ext;
  graphene_rect_t      scale_fig_rect;
  graphene_point_t     p, p1, p2, pt1, pt2;
  GList *              sample_list, *iter;
  double               txt_height;
  guint32              sampler_mode;
  slope_scale_get_figure_rect (scale, &scale_fig_rect);
  cairo_text_extents(cr, "dummy", &txt_ext);
  txt_height = txt_ext.height;

  p.x = priv->min;
  p.y = priv->anchor;
  slope_scale_map(scale, &p1, &p);

  p.x = priv->max;
  p.y = priv->anchor;
  slope_scale_map(scale, &p2, &p);

  if (priv->component & SLOPE_XYAXIS_LINE)
    {
      cairo_new_path(cr);
      gdk_cairo_set_source_rgba (cr, &priv->line_color);
      slope_cairo_line_cosmetic(cr, &p1, &p2, priv->line_width);
      cairo_stroke(cr);

      if (priv->component == SLOPE_XYAXIS_LINE)
        {
          return;
        }
    }

  sampler_mode = slope_sampler_get_mode(priv->sampler);
  if (sampler_mode == SLOPE_SAMPLER_AUTO_DECIMAL)
    {
      slope_sampler_auto_sample_decimal(
          priv->sampler, priv->min, priv->max, (p2.x - p1.x) / 80.0);
    }

  sample_list = slope_sampler_get_sample_list(priv->sampler);
  pt1.y       = graphene_rect_get_y (&scale_fig_rect);
  pt2.y       = graphene_rect_get_y (&scale_fig_rect)
                + graphene_rect_get_height (&scale_fig_rect);
  iter        = sample_list;

  while (iter != NULL)
    {
      SlopeSample *sample;
      graphene_point_t sample_p1, sample_p2;

      sample = SLOPE_XYAXIS_SAMPLE(iter->data);
      iter   = iter->next;
      if (sample->coord < priv->min || sample->coord > priv->max)
        {
          continue;
        }

      p.x = sample->coord;
      p.y = priv->anchor;
      slope_scale_map(scale, &sample_p1, &p);
      sample_p2 = sample_p1;
      sample_p2.y += (priv->component & SLOPE_XYAXIS_TICKS_DOWN) ? -4.0 : 4.0;

      if (priv->component & SLOPE_XYAXIS_GRID)
        {
          cairo_save(cr);
          gdk_cairo_set_source_rgba (cr, &priv->grid_color);
          pt1.x = pt2.x = sample_p1.x;
          slope_cairo_line_cosmetic(cr, &pt1, &pt2, priv->grid_line_width);
          cairo_stroke(cr);
          cairo_restore(cr);
        }
      else if (priv->component & SLOPE_XYAXIS_TICKS_DOWN ||
               priv->component & SLOPE_XYAXIS_TICKS_UP)
        {
          gdk_cairo_set_source_rgba (cr, &priv->line_color);
          slope_cairo_line_cosmetic(
              cr, &sample_p1, &sample_p2, priv->line_width);
          cairo_stroke(cr);
        }

      if (sample->label != NULL && (priv->component & SLOPE_XYAXIS_TICKS_DOWN ||
                                    priv->component & SLOPE_XYAXIS_TICKS_UP))
        {
          cairo_text_extents(cr, sample->label, &txt_ext);
          gdk_cairo_set_source_rgba (cr, &priv->text_color);
          slope_cairo_text(
              cr,
              sample_p1.x - txt_ext.width * 0.5,
              sample_p1.y + ((priv->component & SLOPE_XYAXIS_TICKS_DOWN)
                                 ? txt_height * 1.0
                                 : -txt_height * 0.3),
              sample->label);
        }
    }

  if (priv->title != NULL && (priv->component & SLOPE_XYAXIS_TITLE))
    {
      cairo_text_extents(cr, priv->title, &txt_ext);
      gdk_cairo_set_source_rgba (cr, &priv->title_color);
      if (priv->component & SLOPE_XYAXIS_TICKS_DOWN)
        {
          slope_cairo_text(cr,
                           (p1.x + p2.x - txt_ext.width) / 2.0,
                           p1.y + txt_height * 2.5,
                           priv->title);
        }
      else if (priv->component & SLOPE_XYAXIS_TICKS_UP)
        {
          slope_cairo_text(cr,
                           (p1.x + p2.x - txt_ext.width) / 2.0,
                           p1.y - txt_height * 1.8,
                           priv->title);
        }
      else
        {
          slope_cairo_text(cr,
                           (p1.x + p2.x - txt_ext.width) / 2.0,
                           p1.y + txt_height * 1.3,
                           priv->title);
        }
    }
}

static void _xyaxis_draw_vertical(SlopeXyAxis *self, cairo_t *cr)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  SlopeScale *         scale = slope_item_get_scale(SLOPE_ITEM(self));
  cairo_text_extents_t txt_ext;
  graphene_rect_t      scale_fig_rect;
  graphene_point_t     p, p1, p2, pt1, pt2;
  GList *              sample_list, *iter;
  double               txt_height, max_txt_width = 0.0;
  guint32              sampler_mode;

  slope_scale_get_figure_rect (scale, &scale_fig_rect);
  cairo_text_extents(cr, "dummy", &txt_ext);
  txt_height = txt_ext.height;

  p.x = priv->anchor;
  p.y = priv->min;
  slope_scale_map(scale, &p1, &p);

  p.x = priv->anchor;
  p.y = priv->max;
  slope_scale_map(scale, &p2, &p);

  if (priv->component & SLOPE_XYAXIS_LINE)
    {
      cairo_new_path(cr);
      gdk_cairo_set_source_rgba (cr, &priv->line_color);
      slope_cairo_line_cosmetic(cr, &p1, &p2, priv->line_width);
      cairo_stroke(cr);

      if (priv->component == SLOPE_XYAXIS_LINE)
        {
          return;
        }
    }

  sampler_mode = slope_sampler_get_mode(priv->sampler);
  if (sampler_mode == SLOPE_SAMPLER_AUTO_DECIMAL)
    {
      slope_sampler_auto_sample_decimal(
          priv->sampler, priv->min, priv->max, (p1.y - p2.y) / 80.0);
    }

  sample_list = slope_sampler_get_sample_list(priv->sampler);
  iter        = sample_list;
  pt1.x       = graphene_rect_get_x (&scale_fig_rect);
  pt2.x       = graphene_rect_get_x (&scale_fig_rect)
                + graphene_rect_get_width (&scale_fig_rect);

  while (iter != NULL)
    {
      SlopeSample *sample;
      graphene_point_t sample_p1, sample_p2;

      sample = SLOPE_XYAXIS_SAMPLE(iter->data);
      iter   = iter->next;
      if (sample->coord < priv->min || sample->coord > priv->max)
        {
          continue;
        }

      p.x = priv->anchor;
      p.y = sample->coord;
      slope_scale_map(scale, &sample_p1, &p);
      sample_p2 = sample_p1;
      sample_p2.x += (priv->component & SLOPE_XYAXIS_TICKS_DOWN) ? +4.0 : -4.0;

      if (priv->component & SLOPE_XYAXIS_GRID)
        {
          cairo_save(cr);
          gdk_cairo_set_source_rgba (cr, &priv->grid_color);
          pt1.y = pt2.y = sample_p1.y;
          slope_cairo_line_cosmetic(cr, &pt1, &pt2, priv->grid_line_width);
          cairo_stroke(cr);
          cairo_restore(cr);
        }
      else if (priv->component & SLOPE_XYAXIS_TICKS_DOWN ||
               priv->component & SLOPE_XYAXIS_TICKS_UP)
        {
          gdk_cairo_set_source_rgba (cr, &priv->line_color);
          slope_cairo_line_cosmetic(
              cr, &sample_p1, &sample_p2, priv->line_width);
          cairo_stroke(cr);
        }

      if (sample->label != NULL && (priv->component & SLOPE_XYAXIS_TICKS_DOWN ||
                                    priv->component & SLOPE_XYAXIS_TICKS_UP))
        {
          cairo_text_extents(cr, sample->label, &txt_ext);
          if (txt_ext.width > max_txt_width) max_txt_width = txt_ext.width;
          gdk_cairo_set_source_rgba (cr, &priv->text_color);
          slope_cairo_text(
              cr,
              sample_p1.x + ((priv->component & SLOPE_XYAXIS_TICKS_DOWN)
                                 ? -txt_ext.width - txt_height * 0.3
                                 : +txt_height * 0.3),
              sample_p1.y + txt_height * 0.34,
              sample->label);
        }
    }

  if (priv->title != NULL && (priv->component & SLOPE_XYAXIS_TITLE))
    {
      cairo_save(cr);
      cairo_rotate(cr, -1.5707963267949);
      cairo_text_extents(cr, priv->title, &txt_ext);
      gdk_cairo_set_source_rgba (cr, &priv->title_color);
      if (priv->component & SLOPE_XYAXIS_TICKS_DOWN)
        {
          slope_cairo_text(cr,
                           -((p1.y + p2.y) + txt_ext.width) / 2.0,
                           p1.x - max_txt_width - 1.0 * txt_height,
                           priv->title);
        }
      else if (priv->component & SLOPE_XYAXIS_TICKS_UP)
        {
          slope_cairo_text(cr,
                           -((p1.y + p2.y) + txt_ext.width) / 2.0,
                           p1.x + max_txt_width + 1.6 * txt_height,
                           priv->title);
        }
      else
        {
          slope_cairo_text(cr,
                           -((p1.y + p2.y) + txt_ext.width) / 2.0,
                           p1.x + 0.3 * txt_height,
                           priv->title);
        }
      cairo_restore(cr);
    }
}

void slope_xyaxis_set_position(SlopeXyAxis *self,
                               double       min,
                               double       max,
                               double       anchor)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);

  priv->min    = min;
  priv->max    = max;
  priv->anchor = anchor;
}

void slope_xyaxis_set_components(SlopeXyAxis *self, guint32 components)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->component = components;
}

static void
_xyaxis_get_figure_rect (SlopeItem *self, graphene_rect_t *rect)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (SLOPE_XYAXIS (self));
  SlopeScale *        scale = slope_item_get_scale(self);
  graphene_point_t    p1, p2, p;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      p.x = priv->min;
      p.y = priv->anchor;
      slope_scale_map(scale, &p1, &p);

      p.x = priv->max;
      p.y = priv->anchor;
      slope_scale_map(scale, &p2, &p);

      graphene_rect_init (rect, p1.x, p1.y - 4.0, p2.x - p1.x, 8.0);
    }
  else
    {
      p.x = priv->anchor;
      p.y = priv->min;
      slope_scale_map(scale, &p1, &p);

      p.x = priv->anchor;
      p.y = priv->max;
      slope_scale_map(scale, &p2, &p);

      graphene_rect_init (rect, p1.x - 4.0, p1.y, 8.0, p2.y - p1.y);
    }
}

static void
_xyaxis_get_data_rect (SlopeItem *self, graphene_rect_t *rect)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (SLOPE_XYAXIS (self));

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      graphene_rect_init (rect, priv->min, priv->anchor, priv->max - priv->min, 0.0);
    }
  else
    {
      graphene_rect_init (rect, priv->anchor, priv->min, 0.0, priv->max - priv->min);
    }
}

void slope_xyaxis_set_title(SlopeXyAxis *self, const char *title)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  if (priv->title != NULL)
    {
      g_free(priv->title);
    }
  if (title != NULL)
    {
      priv->title = g_strdup(title);
    }
  else
    {
      priv->title = NULL;
    }
}

SlopeSampler *slope_xyaxis_get_sampler(SlopeXyAxis *self)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  return priv->sampler;
}

const char *slope_xyaxis_get_title(SlopeXyAxis *self)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  return priv->title;
}

gboolean slope_xyaxis_get_selected(SlopeXyAxis *self)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  return priv->selected;
}

void slope_xyaxis_set_selected(SlopeXyAxis *self, gboolean selected)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->selected = selected;
}

void
slope_xyaxis_set_line_color (SlopeXyAxis *self, const GdkRGBA * color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->line_color = *color;
}

void
slope_xyaxis_get_line_color (SlopeXyAxis *self, GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  *color = priv->line_color;
}

void
slope_xyaxis_set_grid_color (SlopeXyAxis *self, const GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->grid_color = *color;
}

void
slope_xyaxis_get_grid_color (SlopeXyAxis *self, GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  *color = priv->grid_color;
}

void
slope_xyaxis_set_title_color (SlopeXyAxis *self, const GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->title_color = *color;
}

void
slope_xyaxis_get_title_color (SlopeXyAxis *self, GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  *color = priv->title_color;
}

void
slope_xyaxis_set_selection_color (SlopeXyAxis *self, const GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  priv->select_rect_color = *color;
}

void
slope_xyaxis_get_selection_color (SlopeXyAxis *self, GdkRGBA *color)
{
  SlopeXyAxisPrivate *priv = slope_xyaxis_get_instance_private (self);
  *color = priv->select_rect_color;
}

/* slope/xyaxis.c */
