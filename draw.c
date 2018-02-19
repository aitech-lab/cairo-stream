#include <stdio.h>
#include <cairo/cairo.h>

#include "app.h"

void draw() {
    unsigned char* data = &buffer[write_frame*frame_size]; 
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w);
    cairo_surface_t* surface = 
        cairo_image_surface_create_for_data(data, CAIRO_FORMAT_RGB24, w, h, stride);
    cairo_t* cr = cairo_create(surface);
    
    // cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 0.4);
    // cairo_paint(cr);
    
    cairo_select_font_face(
        cr, "serif",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);
    
    cairo_set_font_size(cr, 24.0);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

    char text[512];
    cairo_move_to(cr, 10, 30);
    sprintf(text, "buf: %d  fps: %.1f", read_count-write_count, fps);
    cairo_show_text(cr, text);

    cairo_save(cr);
    cairo_scale(cr, 4.0, 4.0);
    cairo_translate(cr, 2, 10);
    
    cairo_set_line_width(cr, 1);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_rectangle(cr, 0, 0, buffer_length, 8);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.5, 0.0, 0.0);
    cairo_move_to(cr, read_frame, 0);
    cairo_line_to(cr, read_frame, 8);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
    cairo_move_to(cr, write_frame, 0);
    cairo_line_to(cr, write_frame, 8);
    cairo_stroke(cr);
    cairo_restore(cr);
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
};

