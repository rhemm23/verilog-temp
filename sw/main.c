#include <uuid/uuid.h>
#include <stdint.h>
#include <stdio.h>

#include "afu_json_info.h"

static void destroy_properties_object(fpga_properties *filter) {
  if (fpgaDestroyProperties(filter) != FPGA_OK) {
    fprintf(stderr, "Failed to destroy properties object\n");
    exit(EXIT_FAILURE);
  }
}

int main() {

  fpga_properties filter = NULL;
  fpga_token token;
  fpga_guid guid;

  uint32_t num_matches = 0;
  
  if (uuid_parse(HELLO_AFU_ID, guid) < 0) {
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

  printf("Number of matches: %d\n", num_matches);

  return 0;
}