; fat12 File Format
BS_OEMName		DB 'SingSing'		;OEM string 8byte
BPB_BytsPerSec	DW	512				;每扇区字节数
BPB_SecPerClus	DB	1				;每簇多少扇区
BPB_RsvdSecCnt	DW	1				;Boot记录占用扇区
BPB_NumFATs		DB	2				;多少FAT表
BPB_RootEntCnt	DW	224				;跟目录文件数最大值
BPB_TotSec16	DW	2880			;逻辑扇区总数
BPB_Media		DB	0xF0			;媒体描述符
BPB_FATSz16		DW	9				;FAT扇区数
BPB_SecPerTrk	DW	18				;每个磁道的扇区数
BPB_NumHeads	DW	2				;磁头数
BPB_HiddSec		DD	0				;隐藏扇区
BPB_TotSec32	DD	0				;TotalSector
BS_DrvNum		DB	0				;中断13驱动器
BS_Reserve		DB	0
BS_BootSig		DB	29h				;扩展引导标记
BS_VolID		DD	0				;卷序列号
BS_VolLab		DB	'           '	;卷标
BS_FileSysType	DB	'FAT12   '		;文件类型 8字节

; 目录区的起始扇区
RootSectors		equ 19
; 文件区的起始扇区
SectorOffset	equ	31
; FAT1表的地址
SectorOfFAT1	equ 1

