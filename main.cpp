#include "machine/rosco_m68k.hpp"
#include "interface/disassembly.hpp"
#include "interface/registers.hpp"
#include "interface/memory.hpp"
#include "interface/terminal.hpp"
#include "interface/button.hpp"

extern "C" {
#include <stdio.h>
#define TB_OPT_TRUECOLOR
#define TB_OPT_EGC
#include <termbox2/termbox.h>
}

typedef struct {
  RoscoM68K*            rosco;
  InterfaceDisassembly* disassembly;
  InterfaceRegisters*   registers;
  InterfaceMemory*      memory;
  InterfaceTerminal*    terminal_a;
  InterfaceButton*      button_step;
  InterfaceButton*      button_multi;
} app_context;

static void uiSingleStep(void* data) {
  app_context* context = (app_context*)data;
  context->rosco->run(1);
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
}

static void uiMultiStep(void* data) {
  app_context* context = (app_context*)data;
  context->rosco->run(5000);
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
}

static void uiRedraw(void* data) {
  app_context* context = (app_context*)data;
  tb_clear();
  context->button_step->update();
  context->button_multi->update();
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
  context->terminal_a->update();
}

static uint8_t rosco_serial_buffer;
static void roscoSerialOutput(uint8_t port, uint8_t transmit_data, void* callback_data) {
  // tb_shutdown();
  // printf("\n\n\nroscoSerialOutput -- port:%d, data:%d (%c)\n\n", port, transmit_data, (char)transmit_data);
  // exit(-1);

  app_context* context = (app_context*)callback_data;
  rosco_serial_buffer = transmit_data;
  context->terminal_a->input(&rosco_serial_buffer, 1);
}

int main(int argc, char** argv) {
  app_context context = { NULL, NULL, NULL, NULL, NULL, NULL };
  context.rosco = new RoscoM68K("rosco_m68k.rom");
  context.rosco->reset();
  context.rosco->duart->setSerialTransmitter(DUART_68681_PORT_A, roscoSerialOutput, &context);

  struct tb_event ui_event;
  tb_init();
  tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
  tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
  tb_set_clear_attrs(0xFFFFFF, 0x444444);

  context.disassembly = new InterfaceDisassembly(context.rosco, 6, 4, 0, 0, 48);
  context.registers   = new InterfaceRegisters(context.rosco, 49, 0);
  context.memory      = new InterfaceMemory(context.rosco, 0, 13, 11);
  context.terminal_a  = new InterfaceTerminal(80, 0, 80, 24);
  context.button_step = new InterfaceButton(0, 24, 16, 3, "&Step", uiSingleStep, &context);
  context.button_multi = new InterfaceButton(18, 24, 16, 3, "Step 5K", uiMultiStep, &context);

  uiRedraw(&context);

  while(true) {
    tb_present();
    tb_poll_event(&ui_event);

    if(ui_event.type == TB_EVENT_RESIZE) {
      uiRedraw(&context);
      continue;
    }

    if(context.button_step->handleEvent(&ui_event)) { continue; }
    if(context.button_multi->handleEvent(&ui_event)) { continue; }
    if(context.memory->handleEvent(&ui_event))      { continue; }

    // global hotkeys
    if(ui_event.ch == 's') { uiSingleStep(&context); continue; }
    if((ui_event.key == TB_KEY_CTRL_Q) && (ui_event.mod & TB_MOD_CTRL)) { break; }
  }

  delete context.button_multi;
  delete context.button_step;
  delete context.memory;
  delete context.registers;
  delete context.disassembly;
  tb_shutdown();
  delete context.rosco;

  printf("\n");
  return 0;
}
