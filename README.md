# OpenParrot MT
Hyperspecialised fork of OpenParrot for Wangan Midnight Maximum Tune.  
Based on [OpenParrot Asakura](https://github.com/ProjectAsakura/OpenParrot).  

## Disclaimer
Please do not ask me to contribute to the TeknoParrot project. I will decline. I will only reconsider if you make the ENTIRETY of TeknoParrot COMPLETELY open-source.

## How to use
Don't, yet. (But you'd be able to use it with TeknoParrotUI or something.)  
No technical support is provided.  

### Need NASM
Download: [NASM](https://github.com/ShiftMediaProject/VSNASM)  
Follow the tutorial to install, and then copy these file to the path:  
> `C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v160\BuildCustomizations\`  
- `nasm.props`
- `nasm.targets`
- `nasm.xml`

### Build setting

- `OpenParrot\src\Functions\Games\ES3X\hook.asm`:  
    1. 属性 ==> 配置属性 ==> 常规 ==> 项类型: `自定义生成工具`  
    2. 自定义生成工具 ==> 常规 ==> 命令行: `<route to nasm.exe> -f win64 <route to hook.asm> -o hook.asm.obj`
    3. 自定义生成工具 ==> 常规 ==> 输出: `hook.asm.obj`


## Special thanks
 - [ProjectAsakura](https://github.com/ProjectAsakura) - original project
 - [Emi (PockyWitch)](https://twitter.com/ChocomintPuppy) - code, protocol analysis, info etc
 - [derole](https://derole.co.uk) - protocol help, info
 - [The Wangan Midnight Emulation Discord](https://discord.gg/r3nbd4x)
 - [FestoROG-XS](https://github.com/FestoROG-XS) - The code of 6RR module comes from him: [AkariaHroptrLoader](https://github.com/FestoROG-XS/AkariaHroptrLoader)