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
  InterfaceButton*      button_run;
  InterfaceButton*      button_reset;
  bool                  free_run;
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
  context->rosco->run(100);
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
}

static void uiFreeRun(void* data) {
  app_context* context = (app_context*)data;
  context->free_run = !context->free_run;
}

static void uiRedraw(void* data) {
  app_context* context = (app_context*)data;
  tb_clear();
  context->button_step->update();
  context->button_multi->update();
  context->button_run->update();
  context->button_reset->update();
  context->disassembly->update();
  context->registers->update();
  context->memory->update();
  context->terminal_a->update();
}

static void uiReset(void* data) {
  app_context* context = (app_context*)data;
  context->rosco->reset();
  uiRedraw(data);
}

static void uiTerminalEvent(uint32_t event_data, void* callback_data) {
  app_context* context = (app_context*)callback_data;
  context->rosco->duart->serialPortReceive(DUART_68681_PORT_A, (uint8_t)event_data);
}

static void roscoSerialOutput(uint8_t port, uint8_t transmit_data, void* callback_data) {
  app_context* context = (app_context*)callback_data;
  context->terminal_a->input(&transmit_data, 1);
}

int main(int argc, char** argv) {
  app_context context = {
    .rosco        = NULL,
    .disassembly  = NULL,
    .registers    = NULL,
    .memory       = NULL,
    .terminal_a   = NULL,
    .button_step  = NULL,
    .button_multi = NULL,
    .button_run   = NULL,
    .button_reset = NULL,
    .free_run     = false,
  };

  try {
    // context.rosco = new RoscoM68K("rosco_m68k.rom");
    // context.rosco = new RoscoM68K("rom.bin");
    context.rosco = new RoscoM68K("./rom/bootrom");
  } catch(const char* error) {
    printf("Exception creating rosco instance: %s\n", error);
    return -1;
  }

  // <test-user-program>
  FILE* fin = fopen("./program/program.bin", "rb");
  uint8_t* destination_a = context.rosco->ram + 0x1338;
  uint8_t* destination_b = context.rosco->ram + 0xBABE;
  uint8_t buffer[512];
  size_t read = fread(buffer, 1, 512, fin);
  while(read) {
    memcpy(destination_a, buffer, read);
    memcpy(destination_b, buffer, read);
    destination_a += read;
    destination_b += read;
    read = fread(buffer, 1, 512, fin);
  }
  fclose(fin);
  // </test-user-program>

  context.rosco->reset();
  context.rosco->duart->setSerialTransmitter(DUART_68681_PORT_A, roscoSerialOutput, &context);

  struct tb_event ui_event;
  tb_init();
  tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
  tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
  tb_set_clear_attrs(0xFFFFFF, 0x444444);

  context.disassembly  = new InterfaceDisassembly(context.rosco, 6, 4, 0, 0, 48);
  context.registers    = new InterfaceRegisters(context.rosco, 49, 0);
  context.memory       = new InterfaceMemory(context.rosco, 0, 13, 11);
  context.terminal_a   = new InterfaceTerminal(80, 0, 80, 24);
  context.button_step  = new InterfaceButton( 0, 24, 16, 3, "&Step",     uiSingleStep, &context);
  context.button_multi = new InterfaceButton(18, 24, 16, 3, "Step &100", uiMultiStep,  &context);
  context.button_run   = new InterfaceButton(36, 24, 16, 3, "&Run",      uiFreeRun,    &context);
  context.button_reset = new InterfaceButton(54, 24, 16, 3, "Reset",     uiReset,      &context);

  context.terminal_a->setEventForwarder(uiTerminalEvent, &context);

  uiRedraw(&context);

  while(true) {
    tb_present();

    bool event = true;
    if(context.free_run) {
      int result = tb_peek_event(&ui_event, 10);
      if((result == TB_ERR_NO_EVENT) || (result == TB_ERR_POLL)) { event = false; }
    } else {
      int result = tb_poll_event(&ui_event);
      if((result == TB_ERR_NO_EVENT) || (result == TB_ERR_POLL)) { event = false; }
    }

    if(event) {
      if(ui_event.type == TB_EVENT_RESIZE) {
        uiRedraw(&context);
        continue;
      }

      if(context.button_step->handleEvent(&ui_event))  { continue; }
      if(context.button_multi->handleEvent(&ui_event)) { continue; }
      if(context.button_run->handleEvent(&ui_event))   { continue; }
      if(context.button_reset->handleEvent(&ui_event)) { continue; }
      if(context.memory->handleEvent(&ui_event))       { continue; }
      if(context.terminal_a->handleEvent(&ui_event))   { continue; }

      // global hotkeys
      if(ui_event.ch == 's') { uiSingleStep(&context); continue; }
      if((ui_event.key == TB_KEY_CTRL_Q) && (ui_event.mod & TB_MOD_CTRL)) { break; }
    }

    if(context.free_run) {
      context.rosco->run(160000);
      context.disassembly->update();
      context.registers->update();
      context.memory->update();
    }
  }

  delete context.button_reset;
  delete context.button_run;
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
