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

#include <math.h>
#include <slope/drawing.h>

#define __SIMILAR_DOUBLE(x1, x2) ((fabs((x2) - (x1)) < 1e-4) ? TRUE : FALSE)

gboolean slope_similar(double x1, double x2)
{
  return __SIMILAR_DOUBLE(x1, x2);
}

void slope_cairo_set_antialias(cairo_t *cr, gboolean antialias)
{
  cairo_set_antialias(
      cr, antialias == TRUE ? CAIRO_ANTIALIAS_SUBPIXEL : CAIRO_ANTIALIAS_NONE);
}

void
slope_cairo_line_cosmetic (cairo_t *cr,
                           const graphene_point_t *p1,
                           const graphene_point_t *p2,
                           double width)
{
  double round_width = round(width);
  cairo_set_line_width(cr, round_width);
  if (__SIMILAR_DOUBLE(round_width, 1.0) ||
      __SIMILAR_DOUBLE(round_width, 3.0) || __SIMILAR_DOUBLE(round_width, 5.0))
    {
      cairo_move_to(
          cr, SLOPE_COORD_TO_PIXEL(p1->x), SLOPE_COORD_TO_PIXEL(p1->y));
      cairo_line_to(
          cr, SLOPE_COORD_TO_PIXEL(p2->x), SLOPE_COORD_TO_PIXEL(p2->y));
    }
  else if (__SIMILAR_DOUBLE(round_width, 2.0) ||
           __SIMILAR_DOUBLE(round_width, 4.0) ||
           __SIMILAR_DOUBLE(round_width, 6.0))
    {
      cairo_move_to(cr, round(p1->x), round(p1->y));
      cairo_line_to(cr, round(p2->x), round(p2->y));
    }
  else
    {
      cairo_move_to(cr, p1->x, p1->y);
      cairo_line_to(cr, p2->x, p2->y);
    }
}

void slope_cairo_rect (cairo_t *cr, const graphene_rect_t *r)
{
  cairo_rectangle (cr, r->origin.x, r->origin.y, r->size.width, r->size.height);
}

void
slope_cairo_round_rect (cairo_t *cr, const graphene_rect_t *rec, double radius)
{
  const double degrees = 0.01745329252; /* ~pi/180 */
  double x      = graphene_rect_get_x (rec);
  double y      = graphene_rect_get_y (rec);
  double width  = graphene_rect_get_width (rec);
  double height = graphene_rect_get_height (rec);

  cairo_new_sub_path(cr);
  cairo_arc(cr,
            x + width - radius,
            y + radius,
            radius,
            -90 * degrees,
            0 * degrees);
  cairo_arc(cr,
            x + width - radius,
            y + height - radius,
            radius,
            0 * degrees,
            90 * degrees);
  cairo_arc(cr,
            x + radius,
            y + height - radius,
            radius,
            90 * degrees,
            180 * degrees);
  cairo_arc(cr,
            x + radius,
            y + radius,
            radius,
            180 * degrees,
            270 * degrees);
  cairo_close_path(cr);
}

void
slope_cairo_draw (cairo_t *cr, const GdkRGBA *stroke, const GdkRGBA *fill)
{
  if (!gdk_rgba_is_clear (stroke) && !gdk_rgba_is_clear (fill))
    {
      gdk_cairo_set_source_rgba (cr, fill);
      cairo_fill_preserve(cr);
      gdk_cairo_set_source_rgba (cr, stroke);
      cairo_stroke(cr);
    }
  else if (!gdk_rgba_is_clear (stroke))
    {
      gdk_cairo_set_source_rgba (cr, stroke);
      cairo_stroke(cr);
    }
  else
    {
      gdk_cairo_set_source_rgba (cr, fill);
      cairo_fill(cr);
    }
}

void slope_cairo_text(cairo_t *cr, double x, double y, const char *utf8)
{
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, utf8);
}

void
slope_cairo_circle (cairo_t *cr, const graphene_point_t *center, double radius)
{
  cairo_move_to(cr, center->x + radius, center->y);
  cairo_arc(cr, center->x, center->y, radius, 0.0, 6.28318530717959);
}

/* slope/drawing.c */
