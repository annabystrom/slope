/*
 * Copyright (C) 2016  Elvis Teixeira
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

#ifndef SLOPE_XYSERIES_H
#define SLOPE_XYSERIES_H

#include <slope/item.h>

#define SLOPE_XYSERIES_TYPE              (slope_xyseries_get_type())
#define SLOPE_XYSERIES(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), SLOPE_XYSERIES_TYPE, SlopeXySeries))
#define SLOPE_XYSERIES_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), SLOPE_XYSERIES_TYPE, SlopeXySeriesClass))
#define SLOPE_IS_XYSERIES(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), SLOPE_XYSERIES_TYPE))
#define SLOPE_IS_XYSERIES_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), SLOPE_XYSERIES_TYPE))
#define SLOPE_XYSERIES_GET_CLASS(obj)    (SLOPE_XYSERIES_CLASS(G_OBJECT_GET_CLASS(obj)))

SLOPE_BEGIN_DECLS

typedef struct
_SlopeXySeries
{
  SlopeItem parent;

  /* Padding to allow adding up to 4 members
     without breaking ABI. */
  gpointer padding[4];
}
SlopeXySeries;


typedef struct
_SlopeXySeriesClass
{
  SlopeItemClass parent_class;

  /* Padding to allow adding up to 4 members
     without breaking ABI. */
  gpointer padding[4];
}
SlopeXySeriesClass;



GType slope_xyseries_get_type (void) G_GNUC_CONST;

SLOPE_END_DECLS

#endif /* SLOPE_XYSERIES_H */
