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

#include <math.h>
#include <slope/slope.h>

GtkWidget * chart;
double *    x, *y;
long n = 50;

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  SlopeScale *scale;
  SlopeItem * series;

  chart = slope_chart_new();

  gtk_application_add_window (app, GTK_WINDOW (chart));

  scale = slope_xyscale_new();
  slope_chart_add_scale(SLOPE_CHART(chart), scale);

  series = slope_xyseries_new_filled("Sine", x, y, n, "kOr");
  slope_scale_add_item(scale, series);

  gtk_window_present (GTK_WINDOW (chart));
}


int main(int argc, char *argv[])
{
  GtkApplication *app;
  int status = 0;

  /* create some sinusoidal data points */
  long k;
  x         = g_malloc(n * sizeof(double));
  y         = g_malloc(n * sizeof(double));
  double dx = 4.0 * G_PI / n;

  for (k = 0; k < n; ++k)
    {
      x[k] = k * dx;
      y[k] = sin(x[k]);
    }

  app = gtk_application_new ("slope.simple", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_free(x);
  g_free(y);

  return 0;
}
