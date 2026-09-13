# 魔兽争霸3 增强补丁（WarcraftHelper 增强）

给 [LoveBeforT/WarcraftHelper](https://github.com/LoveBeforT/WarcraftHelper) 加的两个小功能：

1. **光标锁定**：把鼠标限制在游戏窗口内。窗口化 / 窗口全屏下鼠标不会再跑到另一个屏幕上去。
2. **自动刷新窗口**：每隔 15 秒把游戏窗口宽度抖动 1 像素再复原，清掉高分辨率下的文字重影 / 叠字。

两个功能已经编进 `WarcraftHelper.dll`，**装好开游戏就自动生效，不需要额外运行任何程序**。

---

## 一、这个补丁解决什么问题

魔兽争霸3 在窗口化（或窗口全屏）模式下，鼠标可以自由滑出窗口。
双屏、三屏玩家经常碰到：

- 打团、切屏时鼠标不小心滑到副屏，游戏里英雄就失去控制了；
- 点屏幕边缘想切视角，结果点到了桌面或另一个程序。

独占全屏模式本来就会把鼠标锁住，所以这个问题只在窗口化 / 窗口全屏时出现。
这个补丁就是补上这一段。

另外，显示器高于 1080p 时，窗口化模式下游戏画面偶尔会出现**文字重影 / 叠字**。
上游 WarcraftHelper 本来就有 F7 手动刷新窗口来解决，但每重叠一次就要按一次 F7，很烦。
补丁加了自动版：每 15 秒自己抖一下窗口，F7 手动刷新依然保留。

## 二、功能说明

| 项目 | 说明 |
| --- | --- |
| 锁定范围 | 游戏客户区（不含标题栏和边框） |
| 生效模式 | 窗口化、窗口全屏（独占全屏无需锁定） |
| 自动解锁 | 切到别的程序、游戏弹出对话框、最小化时自动解除，不会把你困在后台窗口里 |
| 自动刷新 | 每 15 秒抖动窗口 1 像素再复原，间隔可配置，设为 0 关闭 |
| 手动刷新 | 游戏中按 F7 立刻刷新一次，和自动刷新互不影响 |
| 开关 | 改 `WarcraftHelper.ini` 即可，游戏运行中也能随时开关 |
| 兼容性 | 不影响原插件的任何其他功能 |

### 配置项

打开游戏目录里的 `WarcraftHelper.ini`，最后几行：

```ini
# 把鼠标限制在游戏窗口内, 防止鼠标滑到另一个屏幕
# 仅窗口化/窗口全屏需要此项; 独占全屏时游戏本来就会锁住鼠标
# 切到其他程序, 或游戏弹出对话框时, 会自动解锁
CursorLock = true

# 每隔多少秒把游戏窗口宽度抖动 1 像素再复原, 用来刷新画面
# 窗口化时偶尔会出现文字重影/叠字, 抖一下窗口即可清掉
# 15 = 每 15 秒自动刷一次; 设为 0 = 关闭此功能
# 另外游戏中按 F7 可以随时手动立刻刷一次(与这个自动刷新互相独立)
AutoRefreshInterval = 15
```

- `CursorLock = true`：开启光标锁定（默认）
- `CursorLock = false`：关闭光标锁定
- `AutoRefreshInterval = 15`：每 15 秒自动刷新一次窗口（默认）
- `AutoRefreshInterval = 0`：关闭自动刷新，只保留 F7 手动刷新
- 填其他数字：按对应秒数刷新

### 自动刷新的安全设计（为什么不会闪屏、不会把窗口弄坏）

1. 只把窗口**变宽 1 像素再复原**，位置不动、高度不动，肉眼看不出窗口在动。
2. 两次移动都用 `MoveWindow(..., FALSE)`，不让系统先擦白再重画，避免整屏闪一下。
3. 还原之后会**回头读一次窗口真实尺寸**，确认已经变回去；没变回去就重试。
   这样不会出现「刷着刷着窗口越来越宽」的情况。
4. 这些情况下不刷新：窗口不是前台窗口、被最小化、不可见、鼠标左键正被按住（防止打断拖拽）。
5. 刷新用的是后台线程，基于时间间隔判断，不会因为游戏卡顿而一次性补刷很多次。

### 光标锁定的安全设计（为什么不会把鼠标锁死）

补丁只在**同时满足**这些条件时才锁定：

1. 配置里 `CursorLock` 开着；
2. 游戏窗口存在、可见、没被最小化；
3. 游戏窗口是**当前前台窗口**（`GetForegroundWindow()` 等于游戏窗口）。

只要 Alt+Tab 切出去、弹出对话框、或者游戏最小化，就会立刻 `ClipCursor(NULL)` 解锁。
`Stop()`（插件卸载时）也会强制解锁一次，绝不留一个锁死的光标给你。

---

## 三、怎么用

### 方式 A：直接用现成的编译结果（推荐，不用装编译工具）

到 [最新版本下载页](https://github.com/Djjywj/WarcraftHelper/releases/latest)
下 `WarcraftHelper-win32.zip`，解压得到 4 个文件：`d3d9.dll`、`WarcraftHelper.dll`、
`WarcraftHelper.ini`、`WarcraftHelperLoader.mix`。

然后：

1. 把这 4 个文件复制到魔兽争霸3 的安装目录（和 `Warcraft III.exe` 同一个文件夹），
   提示覆盖时选「替换」。
2. 启动游戏。用**窗口化**或**窗口全屏**模式进游戏。
3. 鼠标就锁在窗口里了；文字叠字也不用管了，游戏自己每 15 秒刷一次。
4. 想单独关掉某一项：把 ini 里的 `CursorLock` 改成 `false` 关光标锁定；
   把 `AutoRefreshInterval` 改成 `0` 关自动刷新。

> 注意：`WarcraftHelper.ini` 会覆盖你原来的设置。如果你之前改过其他选项，
> 先备份旧文件，装完再把自己的设置填回去。

### 方式 B：自己编译（想改代码就用这个）

需要 **Visual Studio 2022**（安装时勾上「使用 C++ 的桌面开发」）。

源码已经准备好，双击根目录的 **`build.bat`** 即可，脚本会自动：

1. 找 cmake（找不到就用 VS 自带的）；
2. 生成 win32 工程；
3. 用 MinSizeRel 配置编译；
4. 把需要的 4 个文件收进 **`ready`** 文件夹。

编译完把 `ready` 文件夹里的 4 个文件复制到魔兽目录就行。

命令行等价写法（和上游 README 一致）：

```shell
cmake . -A win32 -B build
cmake --build build --config MinSizeRel
```

编译产物在 `build/output/MinSizeRel/` 下（不是上游 README 写的 `build/output/`，
VS 多配置生成器会多一层配置名文件夹）。

> `build` 目录里的 `.exp` `.lib` `.pdb` 是编译中间产物，不用管，只需要上面那 4 个文件。

### 方式 C：云端自动编译（不想装 Visual Studio 就用这个）

推送代码后，GitHub 会在一台 Windows 机器上自动编译（见
`.github/workflows/build.yml`）：

1. 打开仓库页面，点上方 **Actions**；
2. 点最新一次 **windows build** 运行记录；
3. 页面最下方 **Artifacts** 里下载 `WarcraftHelper-win32`；
4. 解压得到那 4 个文件，复制到魔兽目录。

### 自己验证自动刷新逻辑（可选，不用玩游戏）

自动刷新的行为有一套离线测试，不需要魔兽、不需要 Windows：

```shell
bash tests/run.sh
```

它会用假的 Win32 窗口模拟游戏窗口，检查：间隔没到不乱动、到点抖动正好 1 像素、
抖完一定还原回原尺寸、还原失败会重试、切到别的程序/最小化/按住左键时不刷新。
最后打印 `ALL TESTS PASSED` 就是全过。CI 每次推送也会跑这一套。

---

## 四、相比上游改了什么

改动很小，就是新增两个插件 + 注册进配置系统：

| 文件 | 改动 |
| --- | --- |
| `WarcraftHelper/plugin/cursorlock.cpp` / `.hpp` | **新增**，光标锁定实现 |
| `WarcraftHelper/plugin/autowindowrefresh.cpp` / `.hpp` | **新增**，自动刷新窗口实现 |
| `CMakeLists.txt` | 加 `add_compile_options(/utf-8)`（见下方说明） |
| `WarcraftHelper/config/config.cpp` | 注册 `CursorLock` / `AutoRefreshInterval` 配置项 |
| `WarcraftHelper/config/config.hpp` | 加 `m_cursorLock` / `m_autoRefreshInterval` 成员 |
| `WarcraftHelper/helper.cpp` | 启动时挂上 CursorLock 和 AutoWindowRefresh 插件 |
| `WarcraftHelper.ini` | 加 `CursorLock = true`、`AutoRefreshInterval = 15` |
| `WarcraftHelper/plugin/unlockfps.cpp` | 补上 UTF-8 BOM |
| `tests/` | **新增**，自动刷新的离线行为测试（发行版不含，不影响游戏） |
| `.github/workflows/build.yml` | **新增**，云端自动编译出成品 |

上游原有的 **F7 手动刷新窗口**（`windowfixer.cpp`）完全保留，一行没动。
自动刷新和它是两个独立插件，互不干扰。

### 关于 `/utf-8` 和 BOM

上游源码里带中文注释的文件**本来就有 UTF-8 BOM**，MSVC 能正确识别编码。
但 `unlockfps.cpp` 没有 BOM，在中文 Windows（CP936）下，注释末尾的 UTF-8 字节
可能和换行符凑成非法多字节序列，导致**下一行代码被吞掉**，编译报错。

所以补了两件事，双保险：

1. `CMakeLists.txt` 里给 MSVC 加 `/utf-8`；
2. 给 `unlockfps.cpp` 补上 BOM，和仓库里其他源文件保持一致。

这样在中文 Windows 上也能一次编译通过。

---

## 五、更新日志

### 2026-09（本补丁）

- **新增** 光标锁定功能（`CursorLock`），窗口化 / 窗口全屏下鼠标不再跑出游戏窗口
- **新增** 自动刷新窗口功能（`AutoRefreshInterval`，默认 15 秒），自动清掉高分辨率下的文字重影 / 叠字
- **保留** 上游的 F7 手动刷新窗口，和自动刷新互不影响
- **新增** 自动刷新的离线行为测试 `tests/`，覆盖触发时机、原尺寸还原、前台/最小化判定等
- **新增** 云端自动编译（GitHub Actions），不用本地装 Visual Studio 也能出成品
- **修复** 中文 Windows 下 `unlockfps.cpp` 因缺少 BOM 导致的编译错误
- **新增** 一键编译脚本 `build.bat`，自动收集编译产物到 `ready` 文件夹
- **新增** 中文说明文档（本文件）

---

## 六、出处与致谢

### 原始项目

- **项目地址**：<https://github.com/LoveBeforT/WarcraftHelper>
- **原作者**：**[LoveBeforT](https://github.com/LoveBeforT)**
- **发布版本**：v1.18（2026-01-24）
- **许可证**：GPL-3.0

**感谢 LoveBeforT 开发并开源了 WarcraftHelper。** 这个项目解决了魔兽争霸3
在新时代 Windows 上的大量兼容性问题（解锁地图大小、宽屏、解锁 FPS、自动录像等），
没有它这个光标锁定补丁也没有立足之地。本补丁只是站在这个项目基础上的一个小扩展。

也感谢上游 README 里提到的参考项目：

- [YDWE](https://github.com/actboy168/YDWE)
- [RenderEdge](https://github.com/ENAleksey/RenderEdge_Widescreen)

### 本补丁

由 **Djjywj** 在此基础上修改，改动内容见上方「相比上游改了什么」。

### 相关话题

网上关于「魔兽争霸3 鼠标跑出窗口」的讨论，可参考上游 Issues。
本项目 fork 自 [LoveBeforT/WarcraftHelper](https://github.com/LoveBeforT/WarcraftHelper)，
上游 `master` 保持原样，本补丁放在单独分支上，方便随时合并作者的新版本。

---

## 七、许可证

本项目沿用上游的 **GPL-3.0** 许可证，详见 [LICENSE](./LICENSE)。

本补丁属于衍生作品，同样以 GPL-3.0 发布。

---

## 八、以后怎么跟进原作者的新版本

上游更新后（比如作者发了 v1.19），照下面做：

1. 打开 <https://github.com/Djjywj/WarcraftHelper>；
2. 确认左上角分支是 **`cursorlock`**；
3. 点分支旁边的 **「Sync fork」→「Update branch」**；
4. 如果提示有冲突（多半是 `unlockfps.cpp` 那一行 BOM），按页面提示处理，
   或者直接告诉我，我来合。

`master` 分支保持和上游一模一样，没做任何改动，专门当"干净镜子"用。
自己的改动全部在 `cursorlock` 分支上，两边互不干扰。

## 九、反馈

有问题在 <https://github.com/Djjywj/WarcraftHelper/issues> 提。

---

附：原作者 README 原文（LoveBeforT/WarcraftHelper）

> 下面这一段是上游 `readme.md` 的原文，原样保留，方便对照原始功能说明。
> 本项目把原上游文件合并进了本文件（原 `readme.md` 已删除），原因是 Windows
> 不区分文件名大小写，`README.md` 和 `readme.md` 同时存在会导致克隆冲突。

---

**其他推荐的项目**：

​	[重制版转经典版国服语音](https://github.com/LoveBeforT/war3-chinese-voice)

​	[重制版转经典版开场动画](https://github.com/LoveBeforT/war3-reforged-movie)

#### 介绍

魔兽辅助插件，解除地图大小限制，宽屏支持，解锁FPS，自动保存录像，自动显示fps，最大刷新率修复，目录中文名修复(同时也修复了中文名字地图的显示BUG)，自动显血，另外也修复了使用U9魔兽助手时点击地图崩溃问题。

![image-20220910193304584](./.image/pathfix.png)

支持版本：1.20e、1.24e、1.26a、1.27a和1.27b

| 特性             | 1.20e | 1.24e | 1.26a | 1.27a-1.27b |
| ---------------- | ----- | ----- | ----- | ----- |
| 解锁地图大小限制 | √     | √     | √     | √     |
| 宽屏             | √     | √     | √     | √     |
| 解锁fps          | √     | √     | √     | √     |
| 自动保存录像     | √     | √     | √     | √     |
| 自动显示fps      | ×     | √     | √     | √     |
| 最大刷新率修复   | √     | √     | √     | √     |
| 地图目录中文名修复   | √     | √     | √     | √     |
| 自动显血 | √ | 游戏自带 | 游戏自带 | 游戏自带 |
| 字体重叠修复 | √ | √ | √ | √ |
| U9魔兽助手崩溃修复 | × | × | √ | × |
| FPS限制 | × | × | × | √ |

使用方法：把zip压缩包解压后直接放到魔兽目录下，第一次运行使用窗口化模式启动魔兽争霸3，以便插件覆盖注册表中魔兽fps最大值数据。强烈建议使用窗口化模式游玩魔兽。

- 如果你的显示器大于1080p，产生了字体重叠问题，游戏内使用F7键可以刷新窗口，需要窗口化模式。

- 1.20e和1.24e魔兽建议打上d3d8to9补丁，因为新版windows和旧版本魔兽兼容性不好，dx9可以减少卡顿感。

- 录像会自动保存在魔兽replay目录的WHReplay子目录下。

- 所有功能可通过编辑WarcraftHelper.ini配置开关



#### 生成

​	安装cmake，vs2022。

​	执行以下命令生成项目文件：

```shell
cmake . -A win32 -B build
cmake --build build --config MinSizeRel
```

​	生成好的项目文件在build目录下。

​	编译好的文件在build/output目录下。



#### 参考项目

[YDWE](https://github.com/actboy168/YDWE)

[RenderEdge](https://github.com/ENAleksey/RenderEdge_Widescreen)
