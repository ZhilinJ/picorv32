`timescale 1ns/1ps
module tb_pico_flash_cpu;
    reg clk = 0;
    reg run = 0;
    always #5 clk = !clk;
    wire [31:0] addr, din;
    wire [3:0] we;
    wire en, led;
    reg [31:0] dout;
    reg [31:0] memory [0:4095];
    localparam integer LOG_LEN = 43;
    localparam [8*LOG_LEN-1:0] EXPECTED_LOG = "PicoRV32 QSPI demo: BRAM PASS (0x00005A2A)\n";
    integer fd, index, lane;
    reg [7:0] image [0:16383];

    pico_flash_cpu dut (
        .clk(clk), .run(run), .ram_addr(addr), .ram_din(din),
        .ram_dout(dout), .ram_we(we), .ram_en(en), .led(led)
    );

    always @(posedge clk)
        if (en) begin
            dout <= memory[addr[13:2]];
            for (lane = 0; lane < 4; lane = lane + 1)
                if (we[lane])
                    memory[addr[13:2]][8*lane +: 8] <= din[8*lane +: 8];
        end

    initial begin
        fd = $fopen("app.bin", "rb");
        if (!fd)
            $fatal(1, "Cannot open app.bin");
        index = $fread(image, fd);
        $fclose(fd);
        if (index == 0 || index > 16'h3a00)
            $fatal(1, "Invalid image size");
        for (index = 0; index < 4096; index = index + 1)
            memory[index] = {image[4*index+3], image[4*index+2],
                             image[4*index+1], image[4*index]};
        for (index = 'hf00; index < 'hf04; index = index + 1)
            memory[index] = 0;

        repeat (5) @(negedge clk);
        run = 1;
        repeat (5000) begin
            @(negedge clk);
            if (memory['hf00] === 32'h5049434f) begin
                if (memory['hf02] !== 0 || led !== 1 ||
                    memory['hf01] !== LOG_LEN)
                    $fatal(1, "Unexpected LED, exit status or console log: len=%0d", memory['hf01]);
                for (index = 0; index < LOG_LEN; index = index + 1)
                    if (memory['hf04 + index/4][8*(index%4) +: 8] !==
                        EXPECTED_LOG[8*(LOG_LEN-1-index) +: 8])
                        $fatal(1, "Unexpected console byte %0d", index);
                $display("PASS: PicoRV32 ran BRAM firmware, published console log and returned 0");
                $finish;
            end
        end
        $fatal(1, "PicoRV32 did not finish");
    end
endmodule
