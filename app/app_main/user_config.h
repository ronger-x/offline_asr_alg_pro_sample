/**/
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__


//**板级配置选择
/*板级配置更多细节请查看:https://document.chipintelli.com/硬件资料-->模块手册
chipintelli提供的部分开发板和模组，可以通过下面的宏选择，也可以参考开发板的板级配置文
件添加自定义板级配置文件*/

#define CI_CHIP_TYPE                1303    //flash:2MB,SSOP24
#define BOARD_PORT_FILE             "CI-D03GS02S.c"

#ifndef HOST_MIC_USE_NUMBER
#define HOST_MIC_USE_NUMBER            1   //定义mic数量
#endif

//**麦克风电路模式配置
#define MIC_DIFF_SINGLE                0   /*1,单端。0，差分（通用模块都是差分模式，省成本的模块为单端(MICN_L 接GND)时，需要配置为SINGLE）)*/

//**IIS采音功能开关配置
#define USE_IIS1_OUT_PRE_RSLT_AUDIO    0   //1,开启IIS采音功能,可以使用采音板采音,占用PA2~PA6。会多消耗20KB SYS内存 0,关闭IIS采音功能,PA2~PA6可以用于其它功能。
#if USE_IIS1_OUT_PRE_RSLT_AUDIO
//串口采音功能-调试使用
#define USE_UART_SEND_PRE_RSLT_AUDIO                  0                         //是否使能UART将语音数据送出 1-开 0-关
#define USE_UART_SEND_PRE_RSLT_AUDIO_NUMBER   (UART_TypeDef*)(HAL_UART1_BASE)   //用哪个UART口将算法处理后的音频数据送出
#define USE_UART_SEND_PRE_RSLT_AUDIO_BAUD         UART_BaudRate1M               //串口采音波特率
#endif
//**通讯串口配置
#define CONFIG_CI_LOG_UART             HAL_UART0_BASE  //配置log输出使用的串口，请勿与protocol共用同一个串口

#define MSG_COM_USE_UART_EN            0   //0,关闭语音模块通讯协议。1,开启语音模块通讯协议。
#define UART_PROTOCOL_NUMBER           (HAL_UART2_BASE)    //语音模块协议使用的串口，请勿与log共用同一个串口。
#define UART_PROTOCOL_BAUDRATE         (UART_BaudRate9600) //语音模块协议使用的串口波特率。
#define UART_PROTOCOL_VER              2   //语音模块协议版本号:1,一代协议。2,二代协议，255,平台生成协议

#define CLOUD_UART_PROTOCOL_EN         0   //云端协议使能-只有在启英开发者平台做固件配协议能用
#if CLOUD_UART_PROTOCOL_EN
#define CLOUD_CFG_UART_SEND_EN         1   //使能串口发送数据
#define CLOUD_CFG_PLAY_EN              1   //播报音使能
#define CLOUD_CFG_UART_PORT	         ((UART_TypeDef*)(HAL_UART1_BASE))// HAL_UART0_BASE ~ HAL_UART2_BASE，请勿与log共用同一个串口
#define CLOUD_CFG_UART_BAUND_RATE    UART_BaudRate9600
#endif

//**通信串口引脚开漏模式使能配置
//注:推挽模式的IO只能对接3.3V电平的IO，开漏模式可以对接5V电平的IO(外部需要上拉到5V)
#define UART0_PAD_OPENDRAIN_MODE_EN     0   //0,UART0为推挽模式。1,UART0为开漏模式。
#define UART1_PAD_OPENDRAIN_MODE_EN     1   //0,UART1为推挽模式。1,UART1为开漏模式。
#define UART2_PAD_OPENDRAIN_MODE_EN     0   //0,UART2为推挽模式。1,UART2为开漏模式。

//**OTA配置
#define CI_OTA_ENABLE                (0)                 //开启OTA功能，需修改打包升级工具版本，在firmware\config.ini文件中设置成firmware_version=FW_V1，升级与被升级固件均要FW_V1版本工具打包生成
#define CI_OTA_UART_NUMBER           (1)                 //uart0:0  uart1:1  uart2:2 ,引脚需使板级配置文件中复用配置为该UART的引脚 \
                                                         (注意:bootloader中: uart1-(1306使用PB7和PC0, 其他型号使用PA2和PA3) \
                                                                            uart2-(1306使用PB1和PB2, 其他型号使用PA5和PA6)  \
                                                                            请严格按照上面管脚设计，否则会出现管脚不对应无法和 \
                                                                            bootloadr握手成功)
#define CI_OTA_UART_BAUD          (UART_BaudRate115200) //同bootloader握手波特率
//**时钟源配置
#if ((CI_CHIP_TYPE == 1312) || (CI_CHIP_TYPE == 1311) || (CI_CHIP_TYPE == 1303))
#define USE_EXTERNAL_CRYSTAL_OSC        0
#else
#define USE_EXTERNAL_CRYSTAL_OSC        1           //0:使用内部RC作为时钟源。1:使用外部晶振作为时钟源。
#endif

//**波特率自适应功能配置
#if (USE_EXTERNAL_CRYSTAL_OSC == 0)             //使用内部RC时,建议开启波特率自适应(需要电控增加对应支持)。
#define UART_BAUDRATE_CALIBRATE         0       //是否使能波特率自适应功能。
#define BAUDRATE_SYNC_PERIOD            300000  // 波特率同步周期，单位毫秒。
#define BAUDRATE_FAST_SYNC_PERIOD       5000    // 一次波特率同步失败后，下一次同步间隔，单位毫秒。
#define BAUD_CALIBRATE_MAX_WAIT_TIME    400     // 等待反馈包的超时时间，单位毫秒。
#endif

//*红外功能配置
#define USE_IR_ENABEL                   0       //红外功能，1:是 0:否。开启红外功能在使用打包工具升级固件时，请取消勾选“升级完成自动运行”，防止重复烧录\
                                                  红外功能涉及多模型切换和红外码库，firmware文件请参考external\firmware参考\ir(红外)\firmware
#if USE_IR_ENABEL
#define UART_CONTOR_SEND_IR             0       //用通信口进行串口协议控制发红外
#define IR_TEST	                        0       //用通信口进行串口协议的产检
#ifndef USE_NIGHT_LIGHT
#define USE_NIGHT_LIGHT                 1
#endif
#endif
//#define USE_NULL						1
//*语音上传和播放配置(通过串口)
#define VOICE_UPLOAD_BY_UART               (0)                   //通过串口上传语音功能配置，消耗75KB内存
#define AUDIO_COMPRESS_SPEEX_ENABLE         1                    //音频压缩算法。 1-处理使能SPEEX压缩后的数据 0-直接传输PCM数据
#if (VOICE_UPLOAD_BY_UART&&(!USE_NULL||USE_PWK))                 //语音上传功能不能开算法
#error "ONLY USE_NULL WITH NO PWK SUPPORT VOICE_UPLOAD_BY_UART!"
#endif

#define VOICE_PLAY_BY_UART                 (1)                   //通过串口接收音频数据播放配置

#if (VOICE_UPLOAD_BY_UART || VOICE_PLAY_BY_UART)
#define INNER_CODEC_AUDIO_IN_USE_RESAMPLE   0                    //默认打开   0:不开重采样   1：打开重采样
#endif
#if (VOICE_PLAY_BY_UART)
#define PCM_VOICE_PLAY_BY_UART      (0)                          //播放PCM格式 PCM和MP3格式只能选一个
#define MP3_VOICE_PLAY_BY_UART      (!PCM_VOICE_PLAY_BY_UART)    //播放MP3格式 PCM和MP3格式只能选一个，仅支持播放采样率16k且单声道音频数据
#define AUDIO_PLAY_USE_OUTSIDE      (1)                          //启用自定义外部数据源播放
#endif


#define NEWORK_RECV_QUEUE_ENABLE        1    //网络端串口数据入队列
#define CIAS_SIGNAL_MIC_DEMO_ENABLE     1    //单 mic demo功能-离在线
#define CIAS_DOUBLE_MIC_DEMO_ENABLE     (!CIAS_SIGNAL_MIC_DEMO_ENABLE)    //双mic demo功能-离在线
//**语音识别配置
#define USE_SEPARATE_WAKEUP_EN          1       //是否使用独立的唤醒词模型。1:是 0:否。
#define DEFAULT_MODEL_GROUP_ID          1       //模型ID,用于指定上电启动时，默认进入的语言模型。通常0为命令词模型,1为唤醒词模型

#if (!USE_SEPARATE_WAKEUP_EN)
#undef DEFAULT_MODEL_GROUP_ID
#define DEFAULT_MODEL_GROUP_ID          0
#endif

#define PLAY_WELCOME_EN                 1      //是否在启动时播放开机提示音。1:是 0:否。
#define PLAY_ENTER_WAKEUP_EN            1      //是否在唤醒时播放提示音。1:是 0:否。
#define PLAY_EXIT_WAKEUP_EN             1      //是否在切换到只监听唤词状态时播放提示音。1:是 0:否。
#define PLAY_OTHER_CMD_EN               1      //是否在识别到命令词时播放提示音。1:是 0:否。
#define ADAPTIVE_THRESHOLD              0
#define ASR_SKIP_FRAME_CONFIG           0
#define EXIT_WAKEUP_TIME                15*1000   //退出唤醒超时时间,单位毫秒。超过此配置指定的时间长度内没有识别到任何命令词，就会切换到只监听唤词状态。

//**播放器配置
#if (VOICE_PLAY_BY_UART && PCM_VOICE_PLAY_BY_UART)
#define AUDIO_PLAYER_ENABLE             0   //是否启用音频播放器。0:不启用,1:启用。不时用播放功能时，
#else
#define AUDIO_PLAYER_ENABLE             1   //是否启用音频播放器。0:不启用,1:启用。不时用播放功能时，
                                            //关闭此功能可以节省内存空间。
#endif
#define PLAYER_CONTROL_PA               0   //是否有播放器控音频功放开关。0:功放常开,1:播放器在需要播放时才打开,但可能增加一点每一次播放的延迟时间
#define VOLUME_MAX                      7   //设置音量调节的上限值，对应硬件支持的最大音量。
#define VOLUME_MIN                      1   //设置音量调节的下限值，对应最小音量。
#define VOLUME_DEFAULT                  5   //设置音量调节的默认值。

#if AUDIO_PLAYER_ENABLE
#define USE_PROMPT_DECODER              1   //播放器是否支持prompt解码器，1:是 0:否。
#define USE_MP3_DECODER                 1   //为1时加入mp3解码器，1:是 0:否。
#define AUDIO_PLAY_SUPPT_MP3_PROMPT     1   //播放器是否开启mp3提示音，1:是 0:否。
#endif

#define BF_DEEPSE_MODE                  1   //1:全深度分离更耗内存(单双网络都可以用) 0：半深度分离(唤醒词做深度分离，命令词不做，只能用双网络)

#if USE_DEREVERB_MODULE
#define DEREVERB_FREQ_RANGE_INDEX       0  //默认0:算法起效频率160HZ-4800HZ 消耗28KB内存  1: 算法起效频率0-8000HZ 消耗49KB内存     
#endif
#if USE_AEC_MODULE
#define AEC_INTERRUPT_TYPE              2  //默认2: 命令词和唤醒词都可打断  1: 只有命令词能打断   0:只有唤醒词能打断
#endif
//**自学习功能-请在安静环境下，用清晰洪亮的声音进行指令学习，避免环境噪音过大和学习者声音过小导致学习不成功                                            
#if USE_CWSL
#define CWSL_WAKEUP_NUMBER          2           // 可学习的唤醒词数量
#define WAKE_UP_ID                  1           // 学习的唤醒词对应的命令词ID
#define CWSL_REG_TIMES              1           // 学习时 每个词需说几遍，默认 1 遍即可,支持1、2遍,FOR_REG_2TIMES_FLOW_V2 配置 1时,最大支持 3 遍;
#define CWSL_WAKEUP_THRESHOLD       37          // 学习的唤醒词阈值门限，越小越灵敏，默认 37, 最小可配置到 32;
#define CWSL_CMD_THRESHOLD          35          // 学习的命令词阈值门限，越小越灵敏，默认 35，最小可配置到 30；
#define FOR_REG_2TIMES_FLOW_V2      0           // 学习时，说两遍/三遍逻辑，版本二流程，后续均和第一次的比较，一致学习成功，不一致，最多支持说 3 次\
                                                     FOR_REG_2TIMES_FLOW_V2 配置 1时, CWSL_REG_TIMES 必须是 2或3	
#define CWSL_REG_VAD_LEVEL          0           // 学习过程，灵敏度选项配置： 0 低灵敏度，可减少噪声对学习的干扰，需学习过程大声说话；1 高灵敏度，但也可以导致干扰噪声干扰学习
#define CICWSL_TOTAL_TEMPLATE       16          //可存储模板数量

#if (CWSL_REG_TIMES == 3)
#define FOR_REG_2TIMES_FLOW_V2      1
#elif (CWSL_REG_TIMES > 3)
#error "The CWSL_REG_TIMES max 3\n"
#endif
#endif

#if USE_AI_DOA
#define AI_DOA_OUT_TYPE                 3         //doa输出类型：1-唤醒词输出角度  2-命令词输出角度 3-唤醒次和命令词都输出角度
#endif

#if (!USE_BEAMFORMING_MODULE && !USE_DEREVERB_MODULE &&  !USE_AI_DOA) 
#if HOST_MIC_USE_NUMBER == 2
#define USE_DUAL_MIC_ANY                1         //使用任意MIC都可以识别
#endif
#endif

#if USE_BEAMFORMING_MODULE  || USE_AI_DOA || USE_DEREVERB_MODULE || USE_DUAL_MIC_ANY
#define HOST_CODEC_CHA_NUM  2
#define OFFLINE_DUAL_MIC_ALG_SUPPORT    1
#else
#define HOST_CODEC_CHA_NUM              1
#define OFFLINE_DUAL_MIC_ALG_SUPPORT    0
#endif


#if USE_AEC_MODULE
    #if !ON_LINE_SUPPORT
    #define PAUSE_VOICE_IN_WITH_PLAYING  0   //开启aec时关闭
    #endif
    #define IF_JUST_CLOSE_HPOUT_WHILE_NO_PLAY   1

    #define HOST_CODEC_CHA_NUM  2
    #define OFFLINE_DUAL_MIC_ALG_SUPPORT    0

    #if USE_SED_SNORE || USE_SED_CRY
    #error "aec + sed detection alg not support!\n"    //事件检测不支持AEC
    #endif
#endif

#if USE_AEC_MODULE && (USE_BEAMFORMING_MODULE  || USE_AI_DOA || USE_DEREVERB_MODULE || USE_DUAL_MIC_ANY)  //双mic + aec算法必须外部挂codec作为信号回采(推荐7243e)
#define IF_USE_ANOTHER_CODEC_TO_GET_REF 1
#endif
#if USE_BEAMFORMING_MODULE && USE_PWK
 #error "bf + pwk algorithm, not support\n"
#endif
//DOA和AEC在不接外部codec做AEC信号回采，不能同时使用
#if USE_AI_DOA && USE_AEC_MODULE
    #if !IF_USE_ANOTHER_CODEC_TO_GET_REF
    #error "doa + aec algorithm, requires external codec\n"
    #endif
#endif

#if USE_DEREVERB_MODULE || USE_AI_DOA
    #if HOST_MIC_USE_NUMBER == 1
    #error "algorithm requires tow mic, please set HOST_MIC_USE_NUMBER = 2\n"
    #endif
#endif

//离线双麦算法支持 和 在线支持不能同时
#if (OFFLINE_DUAL_MIC_ALG_SUPPORT && ON_LINE_SUPPORT)
#error "error, OFFLINE_DUAL_MIC_ALG_SUPPORT and ON_LINE_SUPPORT can't be toghter\n"
#endif


//AEC软回采 和 在线支持不能同时
#if ON_LINE_SUPPORT && USE_SOFT_AEC_REF
#error "online and soft aec ref can't be toghter\n"
#endif


//AEC软回采 和 重采样
#if USE_SOFT_AEC_REF && INNER_CODEC_AUDIO_IN_USE_RESAMPLE
#error "soft aec ref must be with no resample\n"
#endif


#if !(OFFLINE_DUAL_MIC_ALG_SUPPORT || ON_LINE_SUPPORT) && USE_SOFT_AEC_REF
#error "no two mic alg, no soft aec ref\n"
#endif

#if USE_ALC_AUTO_SWITCH_MODULE && (USE_AI_DOA || USE_DEREVERB_MODULE || USE_BEAMFORMING_MODULE || USE_AEC_MODULE)
#error "two mic alg, no alc auto\n"
#endif

#if USE_VPR || USE_AI_DOA || USE_WMAN_VPR || USE_CWSL || USE_AEC_MODULE || USE_SED_SNORE || USE_SED_CRY || USE_TTS || USE_BEAMFORMING_MODULE
#if MULT_INTENT > 1
 #error "the CI_ALG_TYPE not support mult intent\n"
#endif
#endif

//单麦就近唤醒 和 红外不能与多意图同时使用
#if USE_PWK || USE_IR_ENABEL
#if MULT_INTENT > 1
 #error "pwk and ir, can't be toghter mult intent\n"
#endif
#endif

//单麦就近唤醒 和 动态ALC和深度降噪,自学习，事件检测算法不能同时使用
#if USE_PWK && (USE_ALC_AUTO_SWITCH_MODULE || USE_DENOISE_NN || USE_CWSL || USE_SED_CRY || USE_SED_SNORE)
#error "can't be toghter pwk\n"
#endif

//语音模块协议 和 云端协议使能不能同时使用
#if MSG_COM_USE_UART_EN && CLOUD_UART_PROTOCOL_EN
#error "MSG_COM_USE_UART_EN and CLOUD_UART_PROTOCOL_EN, can't be toghter\n"
#endif
//开启红外功能必须用二代协议
#if USE_IR_ENABEL && (UART_PROTOCOL_VER != 2)
#error "use ir function, UART_PROTOCOL_VER must set 2!\n"
#endif

//BLE相关协议
#if USE_BLE    
#if !USE_NULL || USE_PWK || (MULT_INTENT > 1)
#error "ble not Support algorithm function!\n"
#endif
#define EXTERNAL_CRYSTAL_OSC_FROM_RF           1            //蓝牙端使用外部晶振, 语音端时钟由蓝牙端提供-用户不可修改
#define CIAS_BLE_CONNECT_MODE_ENABLE           1            //ble连接模式使能
#define CIAS_BLE_SCAN_ENABLE                   1            //ble连接模式使能, 同时广播开启扫描功能
#define CIAS_BLE_ADV_GROUP_MODE_ENABEL         0            //ble纯广播模式-不推荐使用
#define CIAS_BLE_DEBUG_ENABLE                  0            //ble 测试模式-客户一般用不上
#define BLE_CONNECT_TIMEOUT                    8            //蓝牙连接超时时间(S),超时会重启蓝牙协议栈,设置为0则表示不进行超时判断
#define BLE_NAME_MAX_LEN                       18           //蓝牙广播名称最大长度
#define RF_RX_TX_MAX_LEN                       20           //蓝牙收发数据最大长度为20,一般不用修改
#define CIAS_PROTOCOL_VER                      1            //和小程序通信协议版本：1-V1.0  2-V1.1
#define CIAS_BLE_USE_DEFAULT_ADV_DATA          1            //和启英小程序配合使用
#define CIAS_BLE_APP_CMD                       1            //小程序调用用户层事件回调函数
#define DEV_DRIVER_EN_ID                   DEV_LIGHT_CONTROL_MAIN_ID

#if     (CIAS_BLE_SCAN_ENABLE == 1 && CIAS_BLE_CONNECT_MODE_ENABLE != 1)
#define CIAS_BLE_CONNECT_MODE_ENABLE           1
#endif
#if CIAS_BLE_CONNECT_MODE_ENABLE && CIAS_BLE_ADV_GROUP_MODE_ENABEL
#error "CIAS_BLE_CONNECT_MODE_ENABLE and CIAS_BLE_ADV_GROUP_MODE_ENABEL ONLY ENABLE ONE"
#endif
#if (CIAS_BLE_DEBUG_ENABLE == 1)                          //开启AT指令
#define CIAS_BLE_AT_ENABLE                     1
#endif 

#endif   //USE_BLE







#endif /* _USER_CONFIG_H_ */
