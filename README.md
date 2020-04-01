# eepromfs

#### 介绍
eepromfs，基于EEPROM的简易类文件的数据读写库，方便做动态功能增减时参数管理。增减参数块类似增减文件，不会对已有数据存储带来影响。EEPROM硬件资源充裕的情况下使用。

#### 软件架构
EPPROM存储区开头存储eepromfs管理相关信息，采用链表方式串联每一个文件，每个文件建立的时候指定名称和大小，新增加的文件自动追加在链表末尾。后续考虑查询方式出V2.0版本。

#### 使用说明
1.  嵌入式arm使用，硬件需要带eeprom，建议采用24C256、24c512之类的大容量芯片。

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 码云特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  码云官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解码云上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是码云最有价值开源项目，是码云综合评定出的优秀开源项目
5.  码云官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  码云封面人物是一档用来展示码云会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
