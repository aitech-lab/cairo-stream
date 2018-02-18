#include <stdio.h>
#include <cairo/cairo.h>

void draw(unsigned char* data, int w, int h, int n, double fps) {
    
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w);
    cairo_surface_t* surface = 
        cairo_image_surface_create_for_data(data, CAIRO_FORMAT_RGB24, w, h, stride);
    cairo_t* cr = cairo_create(surface);
    
    cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 0.4);
    cairo_paint(cr);
    
    cairo_select_font_face(
        cr, "serif",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);
    
    cairo_set_font_size(cr, 24.0);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

    char text[512];
    cairo_move_to(cr, 10, 30);
    sprintf(text, "%d %.2f", n, fps);
    cairo_show_text(cr, text);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
};

