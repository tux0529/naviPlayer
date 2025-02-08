## 简介

naviPlayer是用 [Qt](https://www.qt.io/) + [FFMpeg](https://ffmpeg.org/) 写的 [Subsonic](https://www.subsonic.org/pages/index.jsp) 兼容的音乐客户端，现有优秀客户端有[Sonixd](https://github.com/jeffvli/sonixd)， [Feishin](https://github.com/jeffvli/feishin)， [音流](https://music.aqzscn.cn/)，但他们大都是基于web构建，占用内存太大，我尝试了音流，启动就占用内存200M，稍点几下就奔300M了，特别是专辑列表没有分页，500张专辑拉到底部内存占用就到600M了，我的丐版macmini真的有点吃不消，所以才用Qt开了这个坑。

现有客户端参照 [Navidrome Apps](https://www.navidrome.org/docs/overview/#apps) 和 [Subsonic Apps](https://www.subsonic.org/pages/apps.jsp)

我是个业余玩家，边百度边写，希望能坚持下去。

## Screenshot 2025-02-08

主要框架搭起来了，还有不少功能还有待完善。基本能播放音乐了，因为用了ffmpeg库，支持的音乐格式还比较全。

![预览图](https://tux.red/images/naviPlayer-2024-12-05.webp)

![预览图](https://tux.red/images/Screenshot-2025-02-08.Albums.webp)

![预览图](https://tux.red/images/Screenshot-2025-02-08.Artists.webp)

![预览图](https://tux.red/images/Screenshot-2025-02-08.Genre.webp)

![预览图](https://tux.red/images/Screenshot-2025-02-08.AlbumDetail.webp)


## 下载(Downloads)

项目 [Github](https://github.com/tux0529/naviPlayer) ，因为Github国内访问不是太顺畅，下面同时提供网盘下载。

天翼云盘 [189Cloud](https://cloud.189.cn/web/share?code=yyqQzu22Yzaa)（访问码：ni44）

## 安装(Install)
因macOS安全机制，推荐从Mac App Store安装软件，网上自行下载软件会被隔离，请参照macOS系统自带的使用手册说明安装：

> ### 打开来自身份不明开发者的 Mac App
> 
> 如果你尝试打开可识别开发者未向 Apple 注册的 App，你将看到一个警告对话框。这未必意味着该 App 有问题。例如，某些 App 在开发者 ID 注册开始之前编写。不过，此 App 未经过审核，并且此 App 发布后 macOS 也没有办法检查它是否被修改或者已损坏。
> 
> 恶意软件传播的常见方式是，在 App 中插入有害代码，然后重新分发被感染的 App。因此，来自身份不明开发者的未注册 App 可能会包含有害代码。
> 
> 最安全的方法是，从 Mac App Store 中查找该 App 的更高版本，或者查找替代 App。
> 
> 若要覆盖安全性设置且仍然打开 App，请按照以下步骤操作：
> 
> ---------------
> 
> 1. 在 Mac 上的“访达”  中，找到想要打开的 App。
> 
> 请勿使用启动台来执行此操作。启动台不允许访问快捷菜单。
> 
> 2. 按住 Control 键点按 App 图标，然后从快捷键菜单中选取“打开”。
> 
> 3. 点按“打开”。
> 
> 该 App 将存储为安全性设置的例外情况，今后你将可以通过连按打开它，如同任何注册的 App 那样。
> 
> ---------------
> 
> 【注】你也可以通过点按“隐私与安全性”设置中的“仍要打开”按钮来允许被阻止的 App。此按钮在你尝试打开该 App 后一小时内可用。
> 
> 若要更改可在 Mac 上打开的 App，请选取苹果菜单  >“系统设置”，在边栏中点按“隐私与安全性” ，然后前往“安全性”。（你可能需要向下滚动。）

如果参照上述步骤仍无法打开，请参照[该网页](https://www.macdo.cn/925.html)步骤尝试。

If the program still cannot run after following the above steps, please try following the steps on [this webpage](https://www.macdo.cn/925.html)


