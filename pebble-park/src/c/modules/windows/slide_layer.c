#include "slide_layer.h"

void SlideLayer_update_proc(Layer *layer, GContext *ctx) {
  SlideLayer * slide_layer = *(SlideLayer * *)layer_get_data(layer);
  GRect layer_bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorBlack);

  // Outline
  GRect rect_bounds = layer_bounds;
  graphics_context_set_stroke_width(ctx, POPUP_BORDER_THICKNESS);
  graphics_fill_rect(ctx, rect_bounds, POPUP_CORNER_RADIUS, GCornersAll);
  graphics_draw_round_rect(ctx, rect_bounds, POPUP_CORNER_RADIUS);

  // Text
  GFont font = fonts_get_system_font(POPUP_FONT);
  GRect text_bounds = GRect(layer_bounds.origin.x + POPUP_BORDER_THICKNESS, rect_bounds.origin.y + POPUP_TEXT_Y_OFFSET, layer_bounds.size.w - 2 * POPUP_BORDER_THICKNESS, layer_bounds.size.h - POPUP_TEXT_Y_OFFSET);
  graphics_draw_text(ctx, slide_layer->text, font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void SlideLayer_anim_in_start_handler(Animation *animation, void *context) {

}

void SlideLayer_anim_in_stop_handler(Animation *animation, bool finished, void *context) {
  if (!finished){
    SlideLayer *slide_layer = (SlideLayer *) context;

    SlideLayer_destroy(slide_layer);
  }
}

void SlideLayer_anim_out_start_handler(Animation *animation, void *context) {

}

void SlideLayer_anim_out_stop_handler(Animation *animation, bool finished, void *context) {
  SlideLayer *slide_layer = (SlideLayer *) context;

  SlideLayer_destroy(slide_layer);
}

void SlideLayer_create(SlideLayer *slide_layer) {
  // window_set_background_color(window, GColorClear);
  int height = POPUP_HEIGHT;
  
  Layer *window_layer = window_get_root_layer(slide_layer->window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect layer_bounds = GRect(0, 168, bounds.size.w, height);

  slide_layer->layer = layer_create_with_data(layer_bounds, sizeof(slide_layer));
  *(SlideLayer * *)layer_get_data(slide_layer->layer) = slide_layer;
  layer_set_update_proc(slide_layer->layer, SlideLayer_update_proc);

  // Add to Window
  layer_add_child(window_layer, slide_layer->layer);

  GRect start = GRect(0, bounds.origin.x + bounds.size.h, layer_bounds.size.w, layer_bounds.size.h);
  GRect finish = GRect(0, bounds.origin.x + bounds.size.h - height, layer_bounds.size.w, layer_bounds.size.h);
  PropertyAnimation *move_prop_anim = property_animation_create_layer_frame(slide_layer->layer, &start, &finish);

  Animation *anim_in = property_animation_get_animation(move_prop_anim);
  animation_set_curve(anim_in, AnimationCurveEaseOut);
  animation_set_duration(anim_in, POPUP_SLIDE_DURATION);

  PropertyAnimation *sit_prop_anim = property_animation_create_layer_frame(slide_layer->layer, &finish, &finish);
  Animation *anim_sit = property_animation_get_animation(sit_prop_anim);
  animation_set_duration(anim_sit, POPUP_EXIST_DURATION);
  
  Animation *anim_out = animation_clone(anim_in);
  animation_set_reverse(anim_out, true);

  animation_set_handlers(anim_in, (AnimationHandlers) {
      .started = SlideLayer_anim_in_start_handler,
      .stopped = SlideLayer_anim_in_stop_handler
  }, (void *) slide_layer);

  animation_set_handlers(anim_sit, (AnimationHandlers) {
      .started = SlideLayer_anim_in_start_handler,
      .stopped = SlideLayer_anim_in_stop_handler // Yes this is intentionally the same as in
  }, (void *) slide_layer);

  animation_set_handlers(anim_out, (AnimationHandlers) {
      .started = SlideLayer_anim_out_start_handler,
      .stopped = SlideLayer_anim_out_stop_handler
  }, (void *) slide_layer);

  slide_layer->animation_sequence = animation_sequence_create(anim_in, anim_sit, anim_out, NULL);

  animation_schedule(slide_layer->animation_sequence);
}

void SlideLayer_window_unload(Window *window) {
  SlideLayer *slide_layer = (SlideLayer *) (window_get_user_data(window));

  SlideLayer_destroy(slide_layer);
  window_destroy(window);
}

SlideLayer *SlideLayer_init(Window *window, char *text, void (*destroy_callback)(), void *destroy_context) {
  SlideLayer *slide_layer = NULL;
  slide_layer = malloc(sizeof(SlideLayer));
  if (slide_layer == NULL)
      return NULL;
  slide_layer->window = window;
  strcpy(slide_layer->text, text);
  slide_layer->destroy_callback = destroy_callback;
  slide_layer->destroy_context = destroy_context;

  SlideLayer_create(slide_layer);

//   slide_layer->window = window_create();

//   window_set_window_handlers(slide_layer->window, (WindowHandlers) {
//     .load = SlideLayer_window_load,
//     .unload = SlideLayer_window_unload
//   });
//   window_set_user_data(slide_layer->window, (void *) slide_layer);
//   window_stack_push(slide_layer->window, true);
  
  return slide_layer;
}

void SlideLayer_cancel(SlideLayer *slide_layer) {
  animation_unschedule(slide_layer->animation_sequence);
  slide_layer->animation_sequence = NULL;
}

void SlideLayer_destroy(SlideLayer *slide_layer) {
  slide_layer->destroy_callback(slide_layer->destroy_context);
  layer_destroy(slide_layer->layer);
  free(slide_layer);
}