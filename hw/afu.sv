
`include "platform_if.vh"
`include "afu_json_info.vh"

module afu
  (
    input clk,
    input rst,

    input t_if_ccip_Rx rx,
    output t_if_ccip_Tx tx
  );

  logic [127:0] afu_id = `AFU_ACCEL_UUID;

  reg [63:0] buffer_addr;

  always_ff @(posedge clk or posedge rst) begin
    if (rst) begin
      buffer_addr <= 0;
      tx.c0.hdr <= 0;
      tx.c0.valid <= 0;
      tx.c1.hdr <= 0;
      tx.c1.valid <= 0;
      tx.c2.hdr <= 0;
      tx.c2.mmioRdValid <= 0;
    end else begin

      /*
       * MMIO read request
       */
      if (rx.c0.mmioRdValid) begin

        /*
         * Echo TID
         */
        tx.c2.mmioRdValid <= 1'b1;
        tx.c2.hdr.tid <= mmio_hdr.tid;

        /*
         * Set response data
         */
        case (mmio_hdr.address)

          /*
           * AFU Header
           */
          16'h0000: tx.c2.data <= {
            4'b0001, // Feature type = AFU
            8'b0,    // reserved
            4'b0,    // afu minor revision = 0
            7'b0,    // reserved
            1'b1,    // end of DFH list = 1
            24'b0,   // next DFH offset = 0
            4'b0,    // afu major revision = 0
            12'b0    // feature ID = 0
          };

          /*
           * AFU ID lower
           */
          16'h0002: tx.c2.data <= afu_id[63:0];

          /*
           * AFU ID higher
           */
          16'h0004: tx.c2.data <= afu_id[127:64];

          /*
           * Reserved
           */
          16'h0006: tx.c2.data <= 64'h0;
          16'h0008: tx.c2.data <= 64'h0;

          /*
           * Buffer address
           */
          16'h000A: tx.c2.data <= buffer_addr;

          /*
           * Unknown
           */
          default: tx.c2.data <= 64'h0;
        endcase
      end else begin
        tx.c2.mmioRdValid <= 1'b0;
      end

      /*
       * MMIO write request
       */
      if (rx.c0.mmioWrValid) begin
        case (mmio_hdr.address)
          16'h000A: begin
            // Store buffer address
            buffer_addr <= rx.c0.data;

            // Write 'Hello World!' to buffer
            tx.c1.valid <= 1'b1;
            tx.c1.data <= 512'h0021646c726f7720;
            tx.c1.hdr <= '{
              6'h00,
              eVC_VA,
              1'b1,
              eMOD_CL,
              eCL_LEN_1,
              eREQ_WRLINE_I,
              6'h00,
              t_ccip_clAddr'(rx.c0.data),
              16'h0000
            };
          end
        endcase
      end else begin
        tx.c1.valid <= 1'b0;
      end
    end
  end

endmodule