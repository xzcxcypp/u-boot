/* S3C2440的NAND Flash控制器接口
 * 修改自Linux内核2.6.13文件drivers/mtd/nand/s3c2410.c
 */

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && !defined(CFG_NAND_LEGACY)
#include <s3c2410.h>
#include <nand.h>

#define S3C2440_NFSTAT_READY (1<<0)
#define S3C2440_NFCONT_nFCE  (1<<1)

/* S3C2440 NAND Flash片选函数 */
static void s3c2440_nand_select_chip(struct mtd_info *mtd, int chip)
{
    S3C2440_NAND * const s3c2440nand = S3C2440_GetBase_NAND();
    
    if(chip == -1)
    {
        s3c2440nand->NFCONT |= S3C2440_NFCONT_nFCE;  //禁止片选信号
    }
    else
    {
        s3c2440nand->NFCONT &= ~S3C2440_NFCONT_nFCE;  //使能片选信号
    }
}


/* S3C2440命令和控制函数 */
static void s3c2440_nand_hwcontrol(struct mtd_info *mtd, int cmd,
                unsigned int ctrl)
{
    S3C2440_NAND * const s3c2440nand = S3C2440_GetBase_NAND();
    struct nand_chip *chip = mtd->priv;
    
    switch(cmd)
    {
        case NAND_CTL_SETNCE:
        case NAND_CTL_CLRNCE:
            printf("%s: called for NCE\n", __FUNCTION__);
            break;
            
        case NAND_CTL_SETCLE:
            chip->IO_ADDR_W = (void*)&s3c2440nand->NFCMD;
            break;
        
        case NAND_CTL_SETALE:
            chip->IO_ADDR_W = (void*)&s3c2440nand->NFADDR;
            break;
        
        /* NAND_CTL_CLRCLE, NAND_CTL_CLRALE; */
        default:
            chip->IO_ADDR_W = (void*)&s3c2440nand->NFDATA;
            break;
    }
}

/* 查询NAND Flash状态
 * 返回值，0表示忙，1表示就绪
 */
static int s3c2440_nand_devready(struct mtd_info *mtd)
{
    S3C2440_NAND *const s3c2440nand = S3C2440_GetBase_NAND();
    return (s3c2440nand->NFSTAT & S3C2440_NFSTAT_READY);
}

/* NAND Flash硬件初始化
 * 设置NAND Flash时序，使能NAND Flash控制器
 */
static void s3c24x0_nand_inithw(void)
{
    S3C2440_NAND * const s3c2440nand = S3C2440_GetBase_NAND();
    
#define TACLS  0
#define TWRPH0 4
#define TWRPH1 2
    
    /* 设置时序 */
    s3c2440nand->NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
    /* 初始化ECC，使能NAND Flash控制器，使能片选信号 */
    s3c2440nand->NFCONT = (1<<4)|(0<<1)|(1<<0);
}

/* 被drivers/nand/nand.c调用，初始化NAND Flash硬件，初始化访问接口函数*/
void board_nand_init(struct nand_chip *chip)
{
    S3C2440_NAND * const s3c2440nand = S3C2440_GetBase_NAND();
    
    s3c24x0_nand_inithw();
    
    chip->IO_ADDR_R = (void*)&s3c2440nand->NFDATA;
    chip->IO_ADDR_W = (void*)&s3c2440nand->NFDATA;
    chip->hwcontrol = s3c2440_nand_hwcontrol;
    chip->dev_ready = s3c2440_nand_devready;
    chip->select_chip = s3c2440_nand_select_chip;
    chip->options = 0; //设置位宽等，位宽为8
    chip->eccmode = NAND_ECC_SOFT; //ECC校验方式为软件ECC
}

#endif
