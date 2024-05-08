/*
 * Copyright (C) 2017 Nuno Ferreira
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
#include <slope/slope.h>

SlopeScale * scale;
SlopeItem *  series;
double *     x, *y;
const long   n  = 200;
const double dx = 4.0 * G_PI / 200;
GtkWidget *  chart;

static gboolean timer_callback(GtkWidget *chart)
{
  static long count = 0;
  count++;

  long k;
  for (k = 0; k < n; ++k)
    {
      y[k] = sin(x[k] + 0.1 * count) + sin(1.2 * x[k] - 0.1 * count);
    }

  slope_xyseries_set_data(SLOPE_XYSERIES(series), x, y, n);
  slope_chart_redraw(SLOPE_CHART(chart));
  return TRUE;
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  chart = slope_chart_new();

  gtk_application_add_window (app, GTK_WINDOW (chart));

  scale = slope_xyscale_new();
  slope_chart_add_scale(SLOPE_CHART(chart), scale);

  series = slope_xyseries_new_filled("Wave", x, y, n, "b-");
  slope_scale_add_item(scale, series);

  g_timeout_add(30, (GSourceFunc) timer_callback, (gpointer) chart);

  gtk_window_present (GTK_WINDOW (chart));
}

int main(int argc, char *argv[])
{
  GtkApplication *app;
  int status = 0;

  /* create some sinusoidal data points */
  x = g_malloc(n * sizeof(double));
  y = g_malloc(n * sizeof(double));

  /* the amplitude for the sine wave gives the SCALE of the plot */
  long k;
  for (k = 0; k < n; ++k)
    {
      x[k] = k * dx;
      y[k] = 2.5 * sin(x[k]);
    }

  app = gtk_application_new ("slope.animation", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_free(x);
  g_free(y);

  return status;
}
