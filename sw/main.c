#include <opae/fpga.h>
#include <uuid/uuid.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "afu_json_info.h"

static void close_fpga_handle(fpga_handle handle) {
  if (fpgaClose(handle) != FPGA_OK) {
    fprintf(stderr, "Failed to close fpga handle\n");
    exit(EXIT_FAILURE);
  }
}

static void destroy_token_object(fpga_token *token) {
  if (fpgaDestroyToken(token) != FPGA_OK) {
    fprintf(stderr, "Failed to destroy token object\n");
    exit(EXIT_FAILURE);
  }
}

static void destroy_properties_object(fpga_properties *filter) {
  if (fpgaDestroyProperties(filter) != FPGA_OK) {
    fprintf(stderr, "Failed to destroy properties object\n");
    exit(EXIT_FAILURE);
  }
}

int main() {

  fpga_properties filter = NULL;
  fpga_handle handle;
  fpga_token token;
  fpga_guid guid;

  uint64_t *mmio_space = NULL;
  uint32_t num_matches = 0;
  
  if (uuid_parse(AFU_ACCEL_UUID, guid) < 0) {
    fprintf(stderr, "Error parsing AFU ID\n");
    return EXIT_FAILURE;
  }

  if (fpgaGetProperties(NULL, &filter) != FPGA_OK) {
    fprintf(stderr, "Failed to create properties object\n");
    return EXIT_FAILURE;
  }

  if (fpgaPropertiesSetObjectType(filter, FPGA_ACCELERATOR) != FPGA_OK) {
    destroy_properties_object(&filter);
    fprintf(stderr, "Failed to set object type property\n");
    return EXIT_FAILURE;
  }

  if (fpgaPropertiesSetGUID(filter, guid) != FPGA_OK) {
    destroy_properties_object(&filter);
    fprintf(stderr, "Failed to set guid property\n");
    return EXIT_FAILURE;
  }

  if (fpgaEnumerate(&filter, 1, &token, 1, &num_matches) != FPGA_OK) {
    destroy_properties_object(&filter);
    fprintf(stderr, "Failed to enumerate fpgas\n");
    return EXIT_FAILURE;
  }

  if (fpgaOpen(token, &handle, 0) != FPGA_OK) {
    destroy_token_object(&token);
    destroy_properties_object(&filter);
    fprintf(stderr, "Failed to open fpga\n");
    return EXIT_FAILURE;
  }

  if (fgpaMapMMIO(handle, 0, &mmio_space) != FPGA_OK) {
    close_fpga_handle(handle);
    destroy_token_object(&token);
    destroy_properties_object(&filter);
    fprintf(stderr, "Failed to map mmio space\n");
    return EXIT_FAILURE;
  }

  printf("Low guid bytes: %" PRIx64 "\n", mmio_space[1]);
  printf("High guid bytes: %" PRIx64 "\n", mmio_space[2]);

  close_fpga_handle(handle);
  destroy_token_object(&token);
  destroy_properties_object(&filter);

  return 0;
}