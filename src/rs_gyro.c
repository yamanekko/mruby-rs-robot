
#include "rs_robot.h"

#define AUX_SPI0_CS     0x20204000	//SPI Master Control and Status
#define AUX_SPI0_FIFO   0x20204004	//SPI Master TX and RX FIFOs
#define AUX_SPI0_CLK    0x20204008	//SPI Master Clock Divider
#define AUX_SPI0_DLEN   0x2020400C	//SPI Master Data Length
#define AUX_SPI0_LTOH   0x20204010	//SPI LOSSI mode TOH
#define AUX_SPI0_DC     0x20204014	//SPI DMA DREQ Controls

#define SPI_CS_LEN_LONG		25
#define SPI_CS_DMA_LEN		24
#define SPI_CS_CSPOL2		23
#define SPI_CS_CSPOL1		22
#define SPI_CS_CSPOL0		21
#define SPI_CS_RX			20
#define SPI_CS_RXR			19
#define SPI_CS_TXD			18
#define SPI_CS_RXD			17
#define SPI_CS_DONE			16
#define SPI_CS_LEN			13
#define SPI_CS_REN			12
#define SPI_CS_ADCS			11
#define SPI_CS_INTR			10
#define SPI_CS_INTD		 	9
#define SPI_CS_DMAEN		8
#define SPI_CS_TA		 	7
#define SPI_CS_CSPOL		6
#define SPI_CS_CLEAR_RX		5
#define SPI_CS_CLEAR_TX		4
#define SPI_CS_CPOL		 	3
#define SPI_CS_CPHA		 	2
#define SPI_CS_CS1		 	1
#define SPI_CS_CS0		 	0

#define SPI_CS_TA_ACTIVE (( volatile unsigned int )0x01 << 7)


// SPI0 MODE
#define SPI_CS_MODE_0x (( volatile unsigned int )0x00 << 3) // CPOL = 0
#define SPI_CS_MODE_1x (( volatile unsigned int )0x01 << 3) // CPOL = 1
#define SPI_CS_MODE_x0 (( volatile unsigned int )0x00 << 2) // CPHA = 0
#define SPI_CS_MODE_x1 (( volatile unsigned int )0x01 << 2) // CPHA = 1
#define SPI_CS_MODE_00 ( SPI_CS_MODE_0x | SPI_CS_MODE_x0 )
#define SPI_CS_MODE_01 ( SPI_CS_MODE_0x | SPI_CS_MODE_x1 )
#define SPI_CS_MODE_10 ( SPI_CS_MODE_1x | SPI_CS_MODE_x0 )
#define SPI_CS_MODE_11 ( SPI_CS_MODE_1x | SPI_CS_MODE_x1 )

volatile unsigned int *SPI_CONTROL =  (unsigned int *) AUX_SPI0_CS;
volatile unsigned int *SPI_FIFO    =  (unsigned int *) AUX_SPI0_FIFO;
volatile unsigned int *SPI_CLK     =  (unsigned int *) AUX_SPI0_CLK;


unsigned int ra;
uint16_t flg = 0;

unsigned int spi_read(unsigned int cmd) {
	unsigned int iret;

    *SPI_CONTROL |= SPI_CS_TA_ACTIVE;
    while(1)
    {
    	//送信バッファに空きがあるかどうか
        if(GET32(AUX_SPI0_CS)&(1<<18)) break; //TXD
    }

   *SPI_FIFO = (cmd | 0x80);
   *SPI_FIFO = 0;
    // wait for sent
    while(1) if(GET32(AUX_SPI0_CS)&(1<<16)) break;
    //wait for received
    while(1) if(GET32(AUX_SPI0_CS)&(1<<17)) break;
 //   for(ra2=0;ra2<0x50000;ra2++) dummy(ra2);

    iret = *SPI_FIFO;	 // 読み捨て
    iret = *SPI_FIFO;

    // 通信終了（ TA = 0 ）
    *SPI_CONTROL &= ~SPI_CS_TA_ACTIVE ;

    return iret;
}

void spi_write(unsigned int reg, unsigned int val){
	// TA =1 (start)
	*SPI_CONTROL |= SPI_CS_TA_ACTIVE;
    *SPI_FIFO = reg;
    *SPI_FIFO = val;
    // 送信完了待ち wait for send done signal
    while(1) if(GET32(AUX_SPI0_CS)&(1<<16)) break;
    // TA = 0 (end)
    *SPI_CONTROL &= ~SPI_CS_TA_ACTIVE ;
}



static mrb_value
mrb_rs_gyro_initialize(mrb_state *mrb, mrb_value self)
{
	//TODO パラメータでデフォルト値を変更できるようにする
	// gpio 7, 8, 9, 10, 11

    unsigned int ra;
    unsigned int ra2;
	unsigned int rcv_data = 0;

    ra=GET32(AUX_ENABLES);
    ra|=2; //enable spi0
    PUT32(AUX_ENABLES,ra);

    ra=GET32(GPFSEL0);
    ra&=~(7<<27); //gpio9	//MISO
    ra|=4<<27;    //alt0
    //ra|=1<<27;    //output
    ra&=~(7<<24); //gpio8	//CE0
    ra|=4<<24;    //alt0
    ra&=~(7<<21); //gpio7	//CE1 未使用
    //ra|=4<<21;    //alt0
    ra|=1<<21;    //output
    PUT32(GPFSEL0,ra);
    ra=GET32(GPFSEL1);
    ra&=~(7<<0); //gpio10	//MOSI
    ra|=4<<0;    //alt0
    ra&=~(7<<3); //gpio11	//SCLK
    ra|=4<<3;    //alt0
    PUT32(GPFSEL1,ra);

    *SPI_CONTROL = 0;
	*SPI_CONTROL &= ~(1 << SPI_CS_CSPOL0); //LOW
	*SPI_CONTROL &= ~((1 << SPI_CS_CS0) | (1 << SPI_CS_CS1));//CS0
	//送信バッファ、受信バッファクリア
	*SPI_CONTROL |= (1 << SPI_CS_CLEAR_RX) | (1 << SPI_CS_CLEAR_TX);
	*SPI_CONTROL |= SPI_CS_MODE_00;

    *SPI_CLK = 128;
    // read "Who am I" reg(0x0F) for test
    rcv_data = spi_read(0x0F);

   // 0xD4 is collect
    if(rcv_data != 0xd4){
    	return mrb_false_value();
    }

    // delay 500msec
    for(ra2=0;ra2<0x500000;ra2++) dummy(ra2);

    spi_write(0x20, 0xCF);
    spi_write(0x23, (0x01000000));

    // delay 500msec
    for(ra2=0;ra2<0x500000;ra2++) dummy(ra2);

	return self;
}

static mrb_value
mrb_rs_gyro_read(mrb_state *mrb, mrb_value self)
{
    //read x or y or z axis value
    mrb_int axis;
    int hval,lval,val;
    unsigned int high,low;
    mrb_int ret;
    int offset = 23;

    mrb_get_args(mrb, "i", &axis);
    switch (axis) {
    case 0:	//x		//TODO
        break;
    case 1:	//y
        high = 0x2B;
        low = 0x2A;
        break;
    case 2:	//z		//TODO
        break;
    }

  	hval = spi_read(high);
  	lval = spi_read(low);
    val = hval << 8 | lval;
    if (val>0x8000) {
      	val-=0x10000;
    }
    ret = -1 * val;
    ret = ret - offset;

  	return mrb_fixnum_value(ret);
}


void
mrb_mruby_rs_gyro_gem_init(mrb_state* mrb) {
	struct RClass *gyro;
	gyro = mrb_define_class(mrb, "Gyro", mrb->object_class);

	mrb_define_const(mrb, gyro, "X", mrb_fixnum_value(0));
	mrb_define_const(mrb, gyro, "Y", mrb_fixnum_value(1));
	mrb_define_const(mrb, gyro, "Z", mrb_fixnum_value(2));

	// methods
	mrb_define_method(mrb, gyro, "initialize", mrb_rs_gyro_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, gyro, "read", mrb_rs_gyro_read, MRB_ARGS_REQ(1));

}

void
mrb_mruby_rs_gyro_gem_final(mrb_state* mrb) {
  // finalizer
}
