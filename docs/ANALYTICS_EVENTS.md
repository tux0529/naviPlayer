# naviPlayer 埋点事件清单（SQLite）

## 存储位置
- 数据库：`AppDataLocation/naviPlayer.db`
- 表：`events`
- 字段：
  - `id` INTEGER
  - `ts` INTEGER（秒级 Unix 时间戳）
  - `name` TEXT
  - `payload` TEXT（urlencoded key/value，形如 `k1=v1&k2=v2`）

## 事件列表

### player.mute_toggle
- 触发：点击底部控制栏音量图标切换静音
- payload：
  - `muted`：`0/1`
  - `volume_before_mute`：静音前音量（0-100）

### song.favorite_toggle
- 触发：任意入口切换歌曲收藏（底栏 / 列表收藏列 / 专辑详情）
- payload：
  - `song_id`
  - `star`：`0/1`

### album.favorite_toggle
- 触发：专辑详情页切换专辑收藏
- payload：
  - `album_id`
  - `star`：`0/1`
  - `derived_from_songs`：该专辑是否因“内部存在收藏歌曲”而保持收藏态显示（`0/1`）

### artist.favorite_toggle
- 触发：艺术家表格收藏列切换收藏
- payload：
  - `artist_id`
  - `star`：`0/1`

### artist.search
- 触发：艺术家页搜索框输入变化
- payload：
  - `keyword`

### artist.sort
- 触发：艺术家页点击表头触发排序变化
- payload：
  - `column`：列索引（0=头像，1=艺术家，2=专辑数，3=收藏）
  - `order`：`asc/desc`

## 查询示例（可选）
可使用任意 SQLite 客户端查询：

```sql
SELECT datetime(ts, 'unixepoch', 'localtime') AS t, name, payload
FROM events
ORDER BY id DESC
LIMIT 200;
```

