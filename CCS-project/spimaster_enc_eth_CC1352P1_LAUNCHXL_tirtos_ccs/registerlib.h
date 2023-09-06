/*
 * registerlib.h
 *
 *  Created on: Oct 28, 2019
 *      Author: sramnath
 */

#ifndef REGISTERLIB_H_
#define REGISTERLIB_H_


#define ERXTX_BANK 0x00

#define ERDPTL 0x00
#define ERDPTH 0x01
#define EWRPTL 0x02
#define EWRPTH 0x03
#define ETXSTL 0x04
#define ETXSTH 0x05
#define ETXNDL 0x06
#define ETXNDH 0x07
#define ERXSTL 0x08
#define ERXSTH 0x09
#define ERXNDL 0x0a
#define ERXNDH 0x0b
#define ERXRDPTL 0x0c
#define ERXRDPTH 0x0d


#define EIE   0x1b
#define EIR   0x1c
#define ESTAT 0x1d
#define ECON2 0x1e
#define ECON1 0x1f

#define ESTAT_CLKRDY 0x01
#define ESTAT_TXABRT 0x02

#define ECON1_RXEN   0x04
#define ECON1_TXRTS  0x08

#define ECON2_AUTOINC 0x80
#define ECON2_PKTDEC  0x40

#define EIR_TXIF      0x08



#define RX_BUF_START 0x0000
#define RX_BUF_END   0x0fff

#define TX_BUF_START 0x1200

/* Bank 1 */
#define EPKTCNT_BANK 0x01

#define ERXFCON 0x38   // 0x18 in bank 1
#define EPKTCNT 0x39   // 0x19


/* Bank 2 */
/* MACONx registers are in bank 2 */
#define MACONX_BANK 0x02

#define MACON1  0x40    // 0x00 in bank 2
#define MACON3  0x42    // 0x02 
#define MACON4  0x43    // 0x03
#define MABBIPG 0x44    // 0x04
#define MAIPGL  0x46    // 0x06
#define MAIPGH  0x47    // 0x07
#define MAMXFLL 0x4a    // 0x0a
#define MAMXFLH 0x4b    // 0x0b

#define MIREGADR 0x54
#define MIWRL    0x56
#define MIWRH    0x57
#define MICMD    0x52
#define MIRDL    0x58
#define MIRDH    0x59

#define MACON1_TXPAUS 0x08
#define MACON1_RXPAUS 0x04
#define MACON1_MARXEN 0x01

#define MACON3_PADCFG_FULL 0xe0
#define MACON3_TXCRCEN     0x10
#define MACON3_FRMLNEN     0x02
#define MACON3_FULDPX      0x01

#define MAX_MAC_LENGTH 1518


/* Bank 3 */
#define MAADRX_BANK 0x03

#define MAADR1 0x64 /* MAADR<47:40> */
#define MAADR2 0x65 /* MAADR<39:32> */
#define MAADR3 0x62 /* MAADR<31:24> */
#define MAADR4 0x63 /* MAADR<23:16> */
#define MAADR5 0x60 /* MAADR<15:8> */
#define MAADR6 0x61 /* MAADR<7:0> */
#define MISTAT 0x6a
#define EREVID 0x72
#define ECOCON 0x75

#define ERXFCON_UCEN  0x80
#define ERXFCON_ANDOR 0x40
#define ERXFCON_CRCEN 0x20
#define ERXFCON_MCEN  0x02
#define ERXFCON_BCEN  0x01

/* Physical register mappings */
#define PHCON1  0x00
#define PHSTAT1 0x01
#define PHID1   0x02
#define PHID2   0x3
#define PHCON2  0x10
#define PHSTAT2 0x11
#define PHIE    0x12
#define PHIR    0x13
#define PHLCON  0x14


#endif /* REGISTERLIB_H_ */
