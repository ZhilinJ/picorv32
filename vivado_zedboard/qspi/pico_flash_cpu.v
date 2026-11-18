module pico_flash_cpu (
    input wire clk,
    input wire run,
    output wire [31:0] ram_addr,
    output wire [31:0] ram_din,
    input wire [31:0] ram_dout,
    output wire [3:0] ram_we,
    output wire ram_en,
    output reg led = 0
);
    reg ready = 0;
    wire valid, trap;
    wire [31:0] addr;
    wire [3:0] wstrb;
    wire in_ram = addr[31:14] == 18'h4;
    wire access = run && valid && !ready;
    reg response_ram;
    assign ram_addr = {18'b0, addr[13:2], 2'b00};
    assign ram_en = access && in_ram;
    assign ram_we = ram_en ? wstrb : 4'b0;

    picorv32 #(.PROGADDR_RESET(32'h00010000)) cpu (
        .clk(clk), .resetn(run), .trap(trap),
        .mem_valid(valid), .mem_ready(ready), .mem_addr(addr),
        .mem_wdata(ram_din), .mem_wstrb(wstrb),
        .mem_rdata(response_ram ? ram_dout : 32'b0),
        .irq(32'b0), .pcpi_wr(1'b0), .pcpi_rd(32'b0),
        .pcpi_wait(1'b0), .pcpi_ready(1'b0)
    );

    always @(posedge clk) begin
        ready <= access;
        if (!run || trap)
            led <= 0;
        else if (access && addr == 32'h00002000 && wstrb[0])
            led <= ram_din[0];
        if (access)
            response_ram <= in_ram;
    end
endmodule
