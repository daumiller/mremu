#include <stdio.h>
#include "machine/rosco_m68k.hpp"
#include "interface/disassembly.hpp"
#include "interface/registers.hpp"
#include "interface/button.hpp"
#include "interface/memory.hpp"

extern "C" {
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

typedef struct {
  RoscoM68K*            rosco;
  InterfaceDisassembly* disassembly;
  InterfaceRegisters*   registers;
  InterfaceMemory*      memory;
  InterfaceButton*      button_step;
} app_context;

void uiSingleStep(void* data) {
  app_context* context = (app_context*)data;
  context->rosco->execute();
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
}

void uiRedraw(void* data) {
  app_context* context = (app_context*)data;
  tb_clear();
  context->button_step->update();
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
}

int main(int argc, char** argv) {
  app_context context = { NULL, NULL, NULL, NULL, NULL };
  // context.rosco = new RoscoM68K("rom.bin");
  context.rosco = new RoscoM68K("rosco_m68k.rom");
  context.rosco->reset();

  struct tb_event ui_event;
  tb_init();
  tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
  tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
  tb_set_clear_attrs(0xFFFFFF, 0x444444);

  context.disassembly = new InterfaceDisassembly(context.rosco, 6, 4, 0, 0, 40);
  context.registers   = new InterfaceRegisters(context.rosco, 40, 0);
  context.memory      = new InterfaceMemory(context.rosco, 0, 13, 11);
  context.button_step = new InterfaceButton(0, 24, 16, 3, "&Step", uiSingleStep, &context);

  uiRedraw(&context);

  while(true) {
    tb_present();
    tb_poll_event(&ui_event);

    if(ui_event.type == TB_EVENT_RESIZE) {
      uiRedraw(&context);
      continue;
    }

    if(context.button_step->handleEvent(&ui_event)) { continue; }
    if(context.memory->handleEvent(&ui_event))      { continue; }

    // global hotkeys
    if(ui_event.ch == 's') { uiSingleStep(&context); continue; }
    if((ui_event.key == TB_KEY_CTRL_Q) && (ui_event.mod & TB_MOD_CTRL)) { break; }
  }

  delete context.button_step;
  delete context.memory;
  delete context.registers;
  delete context.disassembly;
  tb_shutdown();
  delete context.rosco;

  printf("\n");
  return 0;
}
