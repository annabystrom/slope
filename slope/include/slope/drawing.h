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

#ifndef SLOPE_DRAWING_H
#define SLOPE_DRAWING_H

#include <graphene.h>
#include <cairo/cairo.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <slope/global.h>

SLOPE_BEGIN_DECLS

#define SLOPE_COORD_TO_PIXEL(coord) (((int) coord) + 0.5)

#define SLOPE_ABS(a) (((a) < 0) ? -(a) : (a))
#define SLOPE_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SLOPE_MAX(a, b) (((a) > (b)) ? (a) : (b))

gboolean slope_similar(double x1, double x2);

void slope_cairo_set_antialias(cairo_t *cr, gboolean antialias);

void slope_cairo_line_cosmetic (cairo_t *cr,
                                const graphene_point_t *p1,
                                const graphene_point_t *p2,
                                double width);

void slope_cairo_rect (cairo_t *cr, const graphene_rect_t *rec);

void slope_cairo_round_rect (cairo_t *cr, const graphene_rect_t *rec, double rad);

void slope_cairo_draw (cairo_t *cr, const GdkRGBA *stroke, const GdkRGBA *fill);

void slope_cairo_text(cairo_t *cr, double x, double y, const char *utf8);

void slope_cairo_circle (cairo_t *cr, const graphene_point_t *center, double radius);

SLOPE_END_DECLS

#endif /* SLOPE_DRAWING_H */
