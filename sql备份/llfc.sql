/*
 Navicat Premium Data Transfer

 Source Server         : chat_server
 Source Server Type    : MySQL
 Source Server Version : 80027 (8.0.27)
 Source Host           : 81.68.86.146:3308
 Source Schema         : llfc

 Target Server Type    : MySQL
 Target Server Version : 80027 (8.0.27)
 File Encoding         : 65001

 Date: 07/02/2026 19:56:11
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for chat_message
-- ----------------------------
DROP TABLE IF EXISTS `chat_message`;
CREATE TABLE `chat_message`  (
  `message_id` bigint UNSIGNED NOT NULL AUTO_INCREMENT,
  `thread_id` bigint UNSIGNED NOT NULL,
  `sender_id` bigint UNSIGNED NOT NULL,
  `recv_id` bigint UNSIGNED NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `status` tinyint NOT NULL DEFAULT 0 COMMENT '0=未读 1=已读 2=撤回',
  `msg_type` tinyint NOT NULL DEFAULT 0 COMMENT '0=文本 1=图片 2=视频 3=文件',
  PRIMARY KEY (`message_id`) USING BTREE,
  INDEX `idx_thread_created`(`thread_id` ASC, `created_at` ASC) USING BTREE,
  INDEX `idx_thread_message`(`thread_id` ASC, `message_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 355 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of chat_message
-- ----------------------------
INSERT INTO `chat_message` VALUES (33, 35, 1002, 1019, '您好,我是llfc', '2025-07-02 06:41:08', '2025-07-24 08:27:45', 2, 0);
INSERT INTO `chat_message` VALUES (34, 35, 1019, 1002, 'We are friends now!', '2025-07-02 06:41:08', '2025-07-24 08:27:39', 2, 0);
INSERT INTO `chat_message` VALUES (35, 35, 1002, 1019, '你好，很高兴认识你', '2025-07-24 21:36:30', '2025-07-24 21:36:30', 2, 0);
INSERT INTO `chat_message` VALUES (36, 35, 1019, 1002, '我也是，很高兴认识你', '2025-07-24 21:36:43', '2025-07-24 21:36:43', 2, 0);
INSERT INTO `chat_message` VALUES (37, 35, 1019, 1002, '中午吃点什么？', '2025-07-25 22:48:04', '2025-07-25 22:48:04', 2, 0);
INSERT INTO `chat_message` VALUES (38, 35, 1002, 1019, '现在外卖三国杀，折扣力度大，赶紧薅羊毛', '2025-07-25 22:48:36', '2025-07-25 22:48:36', 2, 0);
INSERT INTO `chat_message` VALUES (39, 35, 1019, 1002, '刚看了下，我得了一个25减25的券，我去订外卖去了', '2025-07-25 23:09:26', '2025-07-25 23:09:26', 2, 0);
INSERT INTO `chat_message` VALUES (40, 35, 1002, 1019, '我怎么没看到，你在哪个平台？', '2025-07-25 23:09:40', '2025-07-25 23:09:40', 2, 0);
INSERT INTO `chat_message` VALUES (41, 35, 1019, 1002, '我看错了，那不是外卖券，那是超市打折券', '2025-07-25 23:10:20', '2025-07-25 23:10:20', 2, 0);
INSERT INTO `chat_message` VALUES (42, 35, 1019, 1002, '好吧，那我还是去看看外卖活动', '2025-07-27 09:07:21', '2025-07-27 09:07:21', 2, 0);
INSERT INTO `chat_message` VALUES (43, 35, 1002, 1019, '嗯，有什么推荐的外卖可以告诉我', '2025-07-27 09:07:37', '2025-07-27 09:07:37', 2, 0);
INSERT INTO `chat_message` VALUES (44, 35, 1019, 1002, '汉堡炸鸡怎么样？要不要一起订', '2025-07-29 17:40:20', '2025-07-29 17:40:20', 2, 0);
INSERT INTO `chat_message` VALUES (45, 36, 1176, 1175, '您好,我是yulinyi', '2025-08-22 03:07:31', '2025-08-22 03:07:31', 2, 0);
INSERT INTO `chat_message` VALUES (46, 36, 1175, 1176, 'We are friends now!', '2025-08-22 03:07:31', '2025-08-22 03:07:31', 2, 0);
INSERT INTO `chat_message` VALUES (47, 0, 1175, 1176, '你好', '2025-08-22 11:07:45', '2025-08-22 11:07:45', 2, 0);
INSERT INTO `chat_message` VALUES (48, 0, 1176, 1175, '你好', '2025-08-22 11:08:14', '2025-08-22 11:08:14', 2, 0);
INSERT INTO `chat_message` VALUES (49, 0, 1175, 1176, '怎么了', '2025-08-22 11:08:31', '2025-08-22 11:08:31', 2, 0);
INSERT INTO `chat_message` VALUES (50, 0, 1176, 1175, 'xiaodaji', '2025-08-22 11:09:09', '2025-08-22 11:09:09', 2, 0);
INSERT INTO `chat_message` VALUES (51, 0, 1175, 1176, 'lisishei', '2025-08-22 11:09:19', '2025-08-22 11:09:19', 2, 0);
INSERT INTO `chat_message` VALUES (52, 0, 1176, 1175, 'xiaohongshu', '2025-08-22 19:14:59', '2025-08-22 19:14:59', 2, 0);
INSERT INTO `chat_message` VALUES (53, 0, 1175, 1176, '你真是一个', '2025-08-23 14:29:29', '2025-08-23 14:29:29', 2, 0);
INSERT INTO `chat_message` VALUES (54, 0, 1175, 1176, 'ruhe', '2025-08-23 14:33:27', '2025-08-23 14:33:27', 2, 0);
INSERT INTO `chat_message` VALUES (55, 0, 1175, 1176, '666', '2025-08-23 14:33:42', '2025-08-23 14:33:42', 2, 0);
INSERT INTO `chat_message` VALUES (56, 0, 1175, 1176, '6666666', '2025-08-23 14:34:59', '2025-08-23 14:34:59', 2, 0);
INSERT INTO `chat_message` VALUES (57, 0, 1175, 1176, '1314142', '2025-08-23 14:50:01', '2025-08-23 14:50:01', 2, 0);
INSERT INTO `chat_message` VALUES (58, 0, 1175, 1176, '999', '2025-08-23 14:52:50', '2025-08-23 14:52:50', 2, 0);
INSERT INTO `chat_message` VALUES (59, 0, 1175, 1176, '0000000000000', '2025-08-23 15:13:43', '2025-08-23 15:13:43', 2, 0);
INSERT INTO `chat_message` VALUES (60, 0, 1175, 1176, 'aaaaaa', '2025-08-23 15:32:37', '2025-08-23 15:32:37', 2, 0);
INSERT INTO `chat_message` VALUES (61, 0, 1176, 1175, '909090', '2025-08-23 15:50:31', '2025-08-23 15:50:31', 2, 0);
INSERT INTO `chat_message` VALUES (62, 0, 1175, 1176, '007', '2025-08-23 16:00:40', '2025-08-23 16:00:40', 2, 0);
INSERT INTO `chat_message` VALUES (63, 0, 1175, 1176, 'bbbbbbb', '2025-08-23 16:30:29', '2025-08-23 16:30:29', 2, 0);
INSERT INTO `chat_message` VALUES (64, 0, 1175, 1176, 'ccccccc', '2025-08-23 16:43:35', '2025-08-23 16:43:35', 2, 0);
INSERT INTO `chat_message` VALUES (65, 0, 1175, 1176, 'xiaoshengbuxi', '2025-08-23 16:55:03', '2025-08-23 16:55:03', 2, 0);
INSERT INTO `chat_message` VALUES (66, 0, 1175, 1176, 'shengsi', '2025-08-23 17:02:41', '2025-08-23 17:02:41', 2, 0);
INSERT INTO `chat_message` VALUES (67, 0, 1176, 1175, 'whoareyou', '2025-08-23 17:11:41', '2025-08-23 17:11:41', 2, 0);
INSERT INTO `chat_message` VALUES (68, 0, 1175, 1176, 'xiao666', '2025-08-23 17:27:58', '2025-08-23 17:27:58', 2, 0);
INSERT INTO `chat_message` VALUES (69, 0, 1176, 1175, 'niye666', '2025-08-23 17:28:10', '2025-08-23 17:28:10', 2, 0);
INSERT INTO `chat_message` VALUES (70, 0, 1175, 1176, 'liubi', '2025-08-23 17:28:26', '2025-08-23 17:28:26', 2, 0);
INSERT INTO `chat_message` VALUES (71, 0, 1176, 1175, 'xiuer', '2025-08-23 17:28:38', '2025-08-23 17:28:38', 2, 0);
INSERT INTO `chat_message` VALUES (72, 0, 1176, 1175, 'nizhenniubi', '2025-08-23 17:38:43', '2025-08-23 17:38:43', 2, 0);
INSERT INTO `chat_message` VALUES (73, 0, 1175, 1176, '你也是', '2025-08-23 17:39:00', '2025-08-23 17:39:00', 2, 0);
INSERT INTO `chat_message` VALUES (74, 0, 1176, 1175, 'jizhenliu', '2025-08-23 17:39:08', '2025-08-23 17:39:08', 2, 0);
INSERT INTO `chat_message` VALUES (75, 0, 1175, 1176, '666', '2025-08-23 18:05:08', '2025-08-23 18:05:08', 2, 0);
INSERT INTO `chat_message` VALUES (76, 0, 1176, 1175, 'niyeliu', '2025-08-23 18:05:22', '2025-08-23 18:05:22', 2, 0);
INSERT INTO `chat_message` VALUES (77, 0, 1175, 1176, '00000', '2025-08-23 18:06:26', '2025-08-23 18:06:26', 2, 0);
INSERT INTO `chat_message` VALUES (78, 0, 1176, 1175, '77777', '2025-08-23 18:06:37', '2025-08-23 18:06:37', 2, 0);
INSERT INTO `chat_message` VALUES (79, 0, 1176, 1175, '1111111', '2025-08-23 18:07:21', '2025-08-23 18:07:21', 2, 0);
INSERT INTO `chat_message` VALUES (80, 0, 1176, 1175, 'lxxx', '2025-08-23 18:08:12', '2025-08-23 18:08:12', 2, 0);
INSERT INTO `chat_message` VALUES (81, 0, 1176, 1175, 'xxxxxxxx', '2025-08-23 18:18:28', '2025-08-23 18:18:28', 2, 0);
INSERT INTO `chat_message` VALUES (82, 0, 1176, 1175, 'shengshengbuxi', '2025-08-25 11:35:10', '2025-08-25 11:35:10', 2, 0);
INSERT INTO `chat_message` VALUES (83, 0, 1175, 1176, 'llllll', '2025-08-25 11:44:14', '2025-08-25 11:44:14', 2, 0);
INSERT INTO `chat_message` VALUES (84, 0, 1176, 1175, 'mmmmmmm', '2025-08-25 11:44:55', '2025-08-25 11:44:55', 2, 0);
INSERT INTO `chat_message` VALUES (85, 0, 1175, 1176, 'xxx', '2025-08-25 12:27:04', '2025-08-25 12:27:04', 2, 0);
INSERT INTO `chat_message` VALUES (86, 0, 1176, 1175, 'ddd', '2025-08-25 12:27:13', '2025-08-25 12:27:13', 2, 0);
INSERT INTO `chat_message` VALUES (87, 0, 1175, 1176, 'VVVVV', '2025-08-25 12:47:53', '2025-08-25 12:47:53', 2, 0);
INSERT INTO `chat_message` VALUES (88, 0, 1176, 1175, 'BBBBB', '2025-08-25 12:48:12', '2025-08-25 12:48:12', 2, 0);
INSERT INTO `chat_message` VALUES (89, 0, 1175, 1176, 'XXX', '2025-08-25 13:00:19', '2025-08-25 13:00:19', 2, 0);
INSERT INTO `chat_message` VALUES (90, 0, 1176, 1175, 'CCC', '2025-08-25 13:00:41', '2025-08-25 13:00:41', 2, 0);
INSERT INTO `chat_message` VALUES (91, 0, 1176, 1175, 'BBBB', '2025-08-25 13:01:41', '2025-08-25 13:01:41', 2, 0);
INSERT INTO `chat_message` VALUES (92, 0, 1176, 1175, '111', '2025-08-25 13:02:23', '2025-08-25 13:02:23', 2, 0);
INSERT INTO `chat_message` VALUES (93, 0, 1176, 1175, 'dddccc', '2025-08-25 13:02:50', '2025-08-25 13:02:50', 2, 0);
INSERT INTO `chat_message` VALUES (94, 0, 1175, 1176, 'xiaosheng', '2025-08-25 14:21:37', '2025-08-25 14:21:37', 2, 0);
INSERT INTO `chat_message` VALUES (95, 0, 1176, 1175, 'lingyi', '2025-08-25 14:22:07', '2025-08-25 14:22:07', 2, 0);
INSERT INTO `chat_message` VALUES (96, 0, 1175, 1176, 'mmmmmm', '2025-08-25 15:10:57', '2025-08-25 15:10:57', 2, 0);
INSERT INTO `chat_message` VALUES (97, 0, 1176, 1175, '555', '2025-08-25 15:14:32', '2025-08-25 15:14:32', 2, 0);
INSERT INTO `chat_message` VALUES (98, 0, 1175, 1176, 'liyue', '2025-08-25 16:25:15', '2025-08-25 16:25:15', 2, 0);
INSERT INTO `chat_message` VALUES (99, 0, 1176, 1175, 'mengde', '2025-08-25 16:25:38', '2025-08-25 16:25:38', 2, 0);
INSERT INTO `chat_message` VALUES (100, 0, 1175, 1176, 'xiaosheng', '2025-08-25 20:17:43', '2025-08-25 20:17:43', 2, 0);
INSERT INTO `chat_message` VALUES (101, 0, 1176, 1175, 'yulinyi', '2025-08-25 20:19:10', '2025-08-25 20:19:10', 2, 0);
INSERT INTO `chat_message` VALUES (102, 0, 1176, 1175, 'nihao', '2025-08-28 21:44:15', '2025-08-28 21:44:15', 2, 0);
INSERT INTO `chat_message` VALUES (103, 0, 1175, 1176, 'wobuhao', '2025-08-28 21:44:26', '2025-08-28 21:44:26', 2, 0);
INSERT INTO `chat_message` VALUES (104, 0, 1175, 1176, 'nice', '2025-08-28 21:44:34', '2025-08-28 21:44:34', 2, 0);
INSERT INTO `chat_message` VALUES (105, 0, 1176, 1175, 'bunice', '2025-08-28 21:44:41', '2025-08-28 21:44:41', 2, 0);
INSERT INTO `chat_message` VALUES (106, 0, 1176, 1175, 'ddddd', '2025-08-28 21:44:57', '2025-08-28 21:44:57', 2, 0);
INSERT INTO `chat_message` VALUES (107, 0, 1175, 1176, 'ccccc', '2025-08-28 21:45:01', '2025-08-28 21:45:01', 2, 0);
INSERT INTO `chat_message` VALUES (108, 0, 1175, 1176, '你好', '2025-10-09 23:33:53', '2025-10-09 23:33:53', 2, 0);
INSERT INTO `chat_message` VALUES (188, 0, 1175, 1176, 'nihao', '2025-12-20 17:24:49', '2025-12-20 17:24:49', 2, 0);
INSERT INTO `chat_message` VALUES (189, 0, 1175, 1176, 'nishishei', '2025-12-20 17:25:08', '2025-12-20 17:25:08', 2, 0);
INSERT INTO `chat_message` VALUES (190, 0, 1175, 1176, 'nishishei', '2025-12-20 17:25:39', '2025-12-20 17:25:39', 2, 0);
INSERT INTO `chat_message` VALUES (276, 40, 1267, 1268, '123', '2026-01-06 21:55:42', '2026-01-06 21:55:42', 2, 0);
INSERT INTO `chat_message` VALUES (277, 40, 1267, 1268, '4535434534', '2026-01-06 21:55:49', '2026-01-06 21:55:49', 2, 0);
INSERT INTO `chat_message` VALUES (278, 40, 1268, 1267, '123', '2026-01-06 21:57:20', '2026-01-06 21:57:20', 2, 0);
INSERT INTO `chat_message` VALUES (280, 41, 2179, 2181, '您好,我是saheun', '2026-01-30 02:20:52', '2026-01-30 02:20:52', 2, 0);
INSERT INTO `chat_message` VALUES (281, 41, 2181, 2179, 'We are friends now!', '2026-01-30 02:20:52', '2026-01-30 02:20:52', 2, 0);
INSERT INTO `chat_message` VALUES (282, 41, 2179, 2181, '1', '2026-01-30 10:21:04', '2026-01-30 10:21:04', 2, 0);
INSERT INTO `chat_message` VALUES (283, 41, 2179, 2181, '1234567', '2026-01-30 10:21:45', '2026-01-30 10:21:45', 2, 0);
INSERT INTO `chat_message` VALUES (284, 41, 2181, 2179, '123', '2026-01-30 11:44:19', '2026-01-30 11:44:19', 2, 0);
INSERT INTO `chat_message` VALUES (285, 42, 1019, 1005, 'We are friends now!', '2026-01-31 09:34:34', '2026-01-31 09:34:34', 2, 0);
INSERT INTO `chat_message` VALUES (286, 41, 2181, 2179, '测试model气泡', '2026-01-31 17:38:12', '2026-01-31 17:38:12', 2, 0);
INSERT INTO `chat_message` VALUES (287, 44, 1213, 1275, '您好,我是abc123', '2026-01-31 14:19:27', '2026-01-31 14:19:27', 2, 0);
INSERT INTO `chat_message` VALUES (288, 44, 1275, 1213, 'We are friends now!', '2026-01-31 14:19:27', '2026-01-31 14:19:27', 2, 0);
INSERT INTO `chat_message` VALUES (289, 44, 1275, 1213, '测试一下', '2026-02-01 14:16:17', '2026-02-01 14:16:17', 2, 0);
INSERT INTO `chat_message` VALUES (290, 45, 1275, 0, '测试一下这个群聊', '2026-02-01 19:30:40', '2026-02-01 19:30:40', 2, 0);
INSERT INTO `chat_message` VALUES (291, 45, 1213, 0, '好像现在没问题', '2026-02-01 19:30:57', '2026-02-01 19:30:57', 2, 0);
INSERT INTO `chat_message` VALUES (292, 46, 1275, 0, '这个呢', '2026-02-01 19:31:03', '2026-02-01 19:31:03', 2, 0);
INSERT INTO `chat_message` VALUES (293, 46, 1275, 0, 'nice兄弟,已经跑通,接下来是跨服', '2026-02-01 19:31:50', '2026-02-01 19:31:50', 2, 0);
INSERT INTO `chat_message` VALUES (294, 46, 1213, 0, '嗯', '2026-02-01 19:31:55', '2026-02-01 19:31:55', 2, 0);
INSERT INTO `chat_message` VALUES (295, 45, 1275, 0, '来吧,跨服务器的群聊', '2026-02-01 22:18:42', '2026-02-01 22:18:42', 2, 0);
INSERT INTO `chat_message` VALUES (296, 45, 1213, 0, '没收到,完蛋', '2026-02-01 22:20:05', '2026-02-01 22:20:05', 2, 0);
INSERT INTO `chat_message` VALUES (297, 45, 1213, 0, '测试1', '2026-02-01 22:43:53', '2026-02-01 22:43:53', 2, 0);
INSERT INTO `chat_message` VALUES (298, 45, 1213, 0, '测试2', '2026-02-01 22:55:11', '2026-02-01 22:55:11', 2, 0);
INSERT INTO `chat_message` VALUES (299, 45, 1275, 0, '成功收到测试2, 跨服务器的群聊已完成', '2026-02-01 22:55:59', '2026-02-01 22:55:59', 2, 0);
INSERT INTO `chat_message` VALUES (354, 35, 1002, 1019, '053ce9cc-a5ca-4990-bd96-a91e8fdbf359.png', '2026-02-06 17:08:32', '2026-02-06 09:09:04', 2, 1);

-- ----------------------------
-- Table structure for chat_thread
-- ----------------------------
DROP TABLE IF EXISTS `chat_thread`;
CREATE TABLE `chat_thread`  (
  `id` bigint UNSIGNED NOT NULL AUTO_INCREMENT,
  `type` enum('private','group') CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 50 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of chat_thread
-- ----------------------------
INSERT INTO `chat_thread` VALUES (35, 'private', '2025-07-02 06:41:08');
INSERT INTO `chat_thread` VALUES (36, 'private', '2025-08-22 03:07:31');
INSERT INTO `chat_thread` VALUES (37, 'private', '2025-10-03 08:43:18');
INSERT INTO `chat_thread` VALUES (38, 'private', '2025-12-09 10:00:37');
INSERT INTO `chat_thread` VALUES (39, 'private', '2025-12-09 10:00:44');
INSERT INTO `chat_thread` VALUES (40, 'private', '2026-01-06 13:55:39');
INSERT INTO `chat_thread` VALUES (41, 'private', '2026-01-30 02:20:52');
INSERT INTO `chat_thread` VALUES (42, 'private', '2026-01-31 09:34:34');
INSERT INTO `chat_thread` VALUES (43, 'private', '2026-01-31 09:39:00');
INSERT INTO `chat_thread` VALUES (44, 'private', '2026-01-31 14:19:27');
INSERT INTO `chat_thread` VALUES (45, 'group', '2026-01-31 14:41:29');
INSERT INTO `chat_thread` VALUES (46, 'group', '2026-01-31 14:53:29');
INSERT INTO `chat_thread` VALUES (47, 'group', '2026-01-31 14:59:27');
INSERT INTO `chat_thread` VALUES (48, 'group', '2026-01-31 15:11:13');
INSERT INTO `chat_thread` VALUES (49, 'private', '2026-02-05 04:28:26');

-- ----------------------------
-- Table structure for friend
-- ----------------------------
DROP TABLE IF EXISTS `friend`;
CREATE TABLE `friend`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `self_id` int NOT NULL,
  `friend_id` int NOT NULL,
  `back` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `self_friend`(`self_id` ASC, `friend_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 619 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of friend
-- ----------------------------
INSERT INTO `friend` VALUES (55, 1055, 1054, 'sqy');
INSERT INTO `friend` VALUES (56, 1054, 1055, '');
INSERT INTO `friend` VALUES (61, 1012, 1056, 'test28');
INSERT INTO `friend` VALUES (62, 1056, 1012, '');
INSERT INTO `friend` VALUES (63, 1012, 1050, 'test23');
INSERT INTO `friend` VALUES (89, 1038, 1037, 'chae');
INSERT INTO `friend` VALUES (90, 1037, 1038, '');
INSERT INTO `friend` VALUES (127, 2020, 2030, 'yyh');
INSERT INTO `friend` VALUES (128, 2030, 2020, '');
INSERT INTO `friend` VALUES (149, 2182, 2181, 'jmz');
INSERT INTO `friend` VALUES (150, 2181, 2182, '');
INSERT INTO `friend` VALUES (151, 1080, 1079, 'cpt1');
INSERT INTO `friend` VALUES (152, 1079, 1080, '');
INSERT INTO `friend` VALUES (155, 2020, 1090, 'yzmb');
INSERT INTO `friend` VALUES (156, 1090, 2020, '');
INSERT INTO `friend` VALUES (157, 1090, 2030, 'yyh');
INSERT INTO `friend` VALUES (158, 2030, 1090, '');
INSERT INTO `friend` VALUES (159, 2020, 1093, 'yzmb');
INSERT INTO `friend` VALUES (160, 1093, 2020, '');
INSERT INTO `friend` VALUES (161, 1093, 2030, 'yyh');
INSERT INTO `friend` VALUES (162, 2030, 1093, '');
INSERT INTO `friend` VALUES (163, 1085, 1079, 'cpt1');
INSERT INTO `friend` VALUES (164, 1079, 1085, '');
INSERT INTO `friend` VALUES (165, 1085, 1080, 'cpt2');
INSERT INTO `friend` VALUES (166, 1080, 1085, '');
INSERT INTO `friend` VALUES (167, 1087, 1094, 'ming');
INSERT INTO `friend` VALUES (168, 1094, 1087, '');
INSERT INTO `friend` VALUES (185, 1099, 1098, 'Fany');
INSERT INTO `friend` VALUES (186, 1098, 1099, '');
INSERT INTO `friend` VALUES (379, 1096, 1097, 'www');
INSERT INTO `friend` VALUES (380, 1097, 1096, '');
INSERT INTO `friend` VALUES (389, 1019, 1002, 'llfc');
INSERT INTO `friend` VALUES (390, 1002, 1019, 'zack');
INSERT INTO `friend` VALUES (391, 1120, 1122, 'Mrchang');
INSERT INTO `friend` VALUES (392, 1122, 1120, '');
INSERT INTO `friend` VALUES (393, 1121, 1120, 'KNO3');
INSERT INTO `friend` VALUES (394, 1120, 1121, '');
INSERT INTO `friend` VALUES (395, 1121, 1122, 'Mrchang');
INSERT INTO `friend` VALUES (396, 1122, 1121, '');
INSERT INTO `friend` VALUES (397, 1120, 1129, 'maotouying');
INSERT INTO `friend` VALUES (398, 1129, 1120, '');
INSERT INTO `friend` VALUES (401, 1120, 1134, 'gls');
INSERT INTO `friend` VALUES (402, 1134, 1120, '');
INSERT INTO `friend` VALUES (439, 1123, 1120, 'KNO3');
INSERT INTO `friend` VALUES (440, 1120, 1123, '');
INSERT INTO `friend` VALUES (441, 1123, 1126, 'jelly');
INSERT INTO `friend` VALUES (442, 1126, 1123, '');
INSERT INTO `friend` VALUES (469, 1003, 1008, 'zglx2008');
INSERT INTO `friend` VALUES (470, 1008, 1003, '');
INSERT INTO `friend` VALUES (471, 1142, 1114, 'zay');
INSERT INTO `friend` VALUES (472, 1114, 1142, '');
INSERT INTO `friend` VALUES (487, 1154, 1146, 'jmt');
INSERT INTO `friend` VALUES (488, 1146, 1154, '');
INSERT INTO `friend` VALUES (513, 1128, 1149, 'mwt');
INSERT INTO `friend` VALUES (514, 1149, 1128, '');
INSERT INTO `friend` VALUES (517, 1168, 1169, 'cbq');
INSERT INTO `friend` VALUES (518, 1169, 1168, '');
INSERT INTO `friend` VALUES (537, 1171, 1170, 'yanyan');
INSERT INTO `friend` VALUES (538, 1170, 1171, '');
INSERT INTO `friend` VALUES (539, 1175, 1176, '雨天');
INSERT INTO `friend` VALUES (540, 1176, 1175, 'xiao');
INSERT INTO `friend` VALUES (541, 1191, 1189, '我是沈梦洁');
INSERT INTO `friend` VALUES (542, 1189, 1191, '');
INSERT INTO `friend` VALUES (543, 1192, 1189, 'smj_网易');
INSERT INTO `friend` VALUES (549, 1220, 1219, 'fxw');
INSERT INTO `friend` VALUES (550, 1219, 1220, '');
INSERT INTO `friend` VALUES (553, 1221, 1222, 'zhangzhihui  你好');
INSERT INTO `friend` VALUES (554, 1222, 1221, '');
INSERT INTO `friend` VALUES (555, 1221, 1219, 'fxw');
INSERT INTO `friend` VALUES (556, 1219, 1221, '');
INSERT INTO `friend` VALUES (567, 1258, 1257, 'sang');
INSERT INTO `friend` VALUES (568, 1257, 1258, '');
INSERT INTO `friend` VALUES (569, 1259, 1257, 'hpz');
INSERT INTO `friend` VALUES (570, 1257, 1259, '');
INSERT INTO `friend` VALUES (571, 1259, 1258, 'yan');
INSERT INTO `friend` VALUES (572, 1258, 1259, '');
INSERT INTO `friend` VALUES (573, 1250, 1251, 'zg');
INSERT INTO `friend` VALUES (574, 1251, 1250, '');
INSERT INTO `friend` VALUES (579, 1002, 1250, 'lfh');
INSERT INTO `friend` VALUES (580, 1250, 1002, '');
INSERT INTO `friend` VALUES (581, 1060, 1059, 'a');
INSERT INTO `friend` VALUES (582, 1059, 1060, '');
INSERT INTO `friend` VALUES (599, 1267, 1268, '我是1267');
INSERT INTO `friend` VALUES (600, 1268, 1267, '');
INSERT INTO `friend` VALUES (605, 1272, 1271, 'cat');
INSERT INTO `friend` VALUES (606, 1271, 1272, '');
INSERT INTO `friend` VALUES (607, 1274, 1273, 'kawa');
INSERT INTO `friend` VALUES (608, 1273, 1274, '');
INSERT INTO `friend` VALUES (609, 1278, 1257, '郝培智');
INSERT INTO `friend` VALUES (610, 1257, 1278, '');
INSERT INTO `friend` VALUES (611, 1062, 1061, 'lxh1');
INSERT INTO `friend` VALUES (612, 1061, 1062, '');
INSERT INTO `friend` VALUES (613, 2179, 2181, 'jmz');
INSERT INTO `friend` VALUES (614, 2181, 2179, 'saheun');
INSERT INTO `friend` VALUES (615, 1005, 1019, '');
INSERT INTO `friend` VALUES (616, 1019, 1005, 'test');
INSERT INTO `friend` VALUES (617, 1213, 1275, 'suliu');
INSERT INTO `friend` VALUES (618, 1275, 1213, 'abc123');

-- ----------------------------
-- Table structure for friend_apply
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply`;
CREATE TABLE `friend_apply`  (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `from_uid` int NOT NULL,
  `to_uid` int NOT NULL,
  `status` smallint NOT NULL DEFAULT 0,
  `descs` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT '',
  `back_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 451 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of friend_apply
-- ----------------------------
INSERT INTO `friend_apply` VALUES (253, 1002, 1019, 1, '您好,我是llfc', 'zack');
INSERT INTO `friend_apply` VALUES (254, 1122, 1120, 1, '', '');
INSERT INTO `friend_apply` VALUES (255, 1122, 1121, 1, '', '');
INSERT INTO `friend_apply` VALUES (257, 1120, 1121, 1, '', '');
INSERT INTO `friend_apply` VALUES (258, 1129, 1120, 1, '', '');
INSERT INTO `friend_apply` VALUES (260, 1120, 1123, 1, '', '');
INSERT INTO `friend_apply` VALUES (262, 1134, 1120, 1, '', '');
INSERT INTO `friend_apply` VALUES (271, 1126, 1123, 1, '', '');
INSERT INTO `friend_apply` VALUES (272, 1132, 1002, 0, '', '');
INSERT INTO `friend_apply` VALUES (273, 0, 1142, 0, '', '');
INSERT INTO `friend_apply` VALUES (275, 0, 1114, 0, '', '');
INSERT INTO `friend_apply` VALUES (276, 1008, 1003, 1, '', '');
INSERT INTO `friend_apply` VALUES (278, 0, 1058, 0, '', '');
INSERT INTO `friend_apply` VALUES (279, 0, 1055, 0, '', '');
INSERT INTO `friend_apply` VALUES (281, 1114, 1142, 1, '', '');
INSERT INTO `friend_apply` VALUES (306, 1161, 1154, 0, '', '');
INSERT INTO `friend_apply` VALUES (323, 1149, 1128, 1, '', '');
INSERT INTO `friend_apply` VALUES (341, 1169, 1168, 0, '', '');
INSERT INTO `friend_apply` VALUES (353, 1170, 1171, 1, '', '');
INSERT INTO `friend_apply` VALUES (355, 1175, 1176, 0, '您好,我是xiao', 'yulinyi');
INSERT INTO `friend_apply` VALUES (356, 1176, 1175, 1, '您好,我是yulinyi', 'xiao');
INSERT INTO `friend_apply` VALUES (357, 1189, 1191, 1, '', '');
INSERT INTO `friend_apply` VALUES (358, 1189, 1192, 1, '', '');
INSERT INTO `friend_apply` VALUES (382, 1219, 1220, 1, '', '');
INSERT INTO `friend_apply` VALUES (383, 1221, 1220, 0, '', '');
INSERT INTO `friend_apply` VALUES (384, 1222, 1221, 1, '', '');
INSERT INTO `friend_apply` VALUES (385, 1219, 1221, 0, '', '');
INSERT INTO `friend_apply` VALUES (405, 1257, 1258, 1, '', '');
INSERT INTO `friend_apply` VALUES (406, 1257, 1259, 1, '', '');
INSERT INTO `friend_apply` VALUES (407, 1258, 1259, 0, '', '');
INSERT INTO `friend_apply` VALUES (408, 1251, 1250, 0, '', '');
INSERT INTO `friend_apply` VALUES (409, 1250, 1002, 0, '', '');
INSERT INTO `friend_apply` VALUES (411, 3, 1019, 0, '', '');
INSERT INTO `friend_apply` VALUES (412, 1060, 1002, 0, '', '');
INSERT INTO `friend_apply` VALUES (413, 1060, 1019, 0, '', '');
INSERT INTO `friend_apply` VALUES (414, 1059, 1002, 0, '', '');
INSERT INTO `friend_apply` VALUES (415, 1059, 1060, 1, '', '');
INSERT INTO `friend_apply` VALUES (433, 1268, 1252, 0, '', '');
INSERT INTO `friend_apply` VALUES (434, 1268, 1267, 1, '', '');
INSERT INTO `friend_apply` VALUES (435, 1267, 1268, 1, '', '');
INSERT INTO `friend_apply` VALUES (437, 1271, 1272, 1, '', '');
INSERT INTO `friend_apply` VALUES (438, 1273, 1274, 1, '', '');
INSERT INTO `friend_apply` VALUES (439, 1257, 1278, 1, '', '');
INSERT INTO `friend_apply` VALUES (440, 1008, 1005, 0, '', '');
INSERT INTO `friend_apply` VALUES (442, 1005, 1019, 1, '', '');
INSERT INTO `friend_apply` VALUES (443, 1062, 1019, 0, '', '');
INSERT INTO `friend_apply` VALUES (445, 1061, 1062, 1, '', '');
INSERT INTO `friend_apply` VALUES (446, 2179, 2181, 1, '您好,我是saheun', 'jmz');
INSERT INTO `friend_apply` VALUES (448, 1213, 1275, 1, '您好,我是abc123', 'suliu');
INSERT INTO `friend_apply` VALUES (449, 1280, 1002, 0, '', '');

-- ----------------------------
-- Table structure for group_chat
-- ----------------------------
DROP TABLE IF EXISTS `group_chat`;
CREATE TABLE `group_chat`  (
  `thread_id` bigint UNSIGNED NOT NULL COMMENT '引用chat_thread.id',
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '群聊名称',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`thread_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of group_chat
-- ----------------------------
INSERT INTO `group_chat` VALUES (45, NULL, '2026-01-31 14:41:30');
INSERT INTO `group_chat` VALUES (46, NULL, '2026-01-31 14:53:29');
INSERT INTO `group_chat` VALUES (47, NULL, '2026-01-31 14:59:28');
INSERT INTO `group_chat` VALUES (48, NULL, '2026-01-31 15:11:14');

-- ----------------------------
-- Table structure for group_chat_member
-- ----------------------------
DROP TABLE IF EXISTS `group_chat_member`;
CREATE TABLE `group_chat_member`  (
  `thread_id` bigint UNSIGNED NOT NULL COMMENT '引用 group_chat_thread.thread_id',
  `user_id` bigint UNSIGNED NOT NULL COMMENT '引用 user.user_id',
  `role` tinyint NOT NULL DEFAULT 0 COMMENT '0=普通成员,1=管理员,2=创建者',
  `joined_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `muted_until` timestamp NULL DEFAULT NULL COMMENT '如果被禁言，可存到什么时候',
  PRIMARY KEY (`thread_id`, `user_id`) USING BTREE,
  INDEX `idx_user_threads`(`user_id` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of group_chat_member
-- ----------------------------
INSERT INTO `group_chat_member` VALUES (45, 1213, 0, '2026-01-31 14:41:30', NULL);
INSERT INTO `group_chat_member` VALUES (45, 1275, 2, '2026-01-31 14:41:30', NULL);
INSERT INTO `group_chat_member` VALUES (46, 1213, 0, '2026-01-31 14:53:29', NULL);
INSERT INTO `group_chat_member` VALUES (46, 1275, 2, '2026-01-31 14:53:29', NULL);
INSERT INTO `group_chat_member` VALUES (47, 1213, 0, '2026-01-31 14:59:28', NULL);
INSERT INTO `group_chat_member` VALUES (47, 1275, 2, '2026-01-31 14:59:28', NULL);
INSERT INTO `group_chat_member` VALUES (48, 1213, 0, '2026-01-31 15:11:14', NULL);
INSERT INTO `group_chat_member` VALUES (48, 1275, 2, '2026-01-31 15:11:14', NULL);

-- ----------------------------
-- Table structure for private_chat
-- ----------------------------
DROP TABLE IF EXISTS `private_chat`;
CREATE TABLE `private_chat`  (
  `thread_id` bigint UNSIGNED NOT NULL COMMENT '引用chat_thread.id',
  `user1_id` bigint UNSIGNED NOT NULL,
  `user2_id` bigint UNSIGNED NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`thread_id`) USING BTREE,
  UNIQUE INDEX `uniq_private_thread`(`user1_id` ASC, `user2_id` ASC) USING BTREE,
  INDEX `idx_private_user1_thread`(`user1_id` ASC, `thread_id` ASC) USING BTREE,
  INDEX `idx_private_user2_thread`(`user2_id` ASC, `thread_id` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of private_chat
-- ----------------------------
INSERT INTO `private_chat` VALUES (35, 1019, 1002, '2025-07-02 06:41:08');
INSERT INTO `private_chat` VALUES (36, 1175, 1176, '2025-08-22 03:07:31');
INSERT INTO `private_chat` VALUES (37, 0, 1217, '2025-10-03 08:43:18');
INSERT INTO `private_chat` VALUES (38, 1093, 2020, '2025-12-09 10:00:37');
INSERT INTO `private_chat` VALUES (39, 2020, 2030, '2025-12-09 10:00:44');
INSERT INTO `private_chat` VALUES (40, 1267, 1268, '2026-01-06 13:55:39');
INSERT INTO `private_chat` VALUES (41, 2181, 2179, '2026-01-30 02:20:52');
INSERT INTO `private_chat` VALUES (42, 1019, 1005, '2026-01-31 09:34:34');
INSERT INTO `private_chat` VALUES (43, 0, 2181, '2026-01-31 09:39:01');
INSERT INTO `private_chat` VALUES (44, 1275, 1213, '2026-01-31 14:19:27');
INSERT INTO `private_chat` VALUES (49, 0, 1291, '2026-02-05 04:28:26');

-- ----------------------------
-- Table structure for test
-- ----------------------------
DROP TABLE IF EXISTS `test`;
CREATE TABLE `test`  (
  `id` int NOT NULL,
  `product_no` varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT NULL,
  `name` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT NULL,
  `price` decimal(10, 2) NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `index_product_no_name`(`product_no` ASC, `name` ASC) USING BTREE,
  INDEX `index_id_name`(`id` ASC, `name` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of test
-- ----------------------------
INSERT INTO `test` VALUES (1, '0001', 'apple', 6.00);
INSERT INTO `test` VALUES (2, '0002', 'banana', 2.00);
INSERT INTO `test` VALUES (3, '0003', 'orange', 3.00);
INSERT INTO `test` VALUES (4, '0004', 'iphone13', 5000.00);
INSERT INTO `test` VALUES (5, '0005', 'ipad8', 3500.00);
INSERT INTO `test` VALUES (6, '0006', 'macbookpro', 10000.00);
INSERT INTO `test` VALUES (7, '0007', 'ps5', 4000.00);
INSERT INTO `test` VALUES (8, '0008', 'grape', 10.00);
INSERT INTO `test` VALUES (9, '0009', 'watermelon', 40.00);
INSERT INTO `test` VALUES (10, '0010', 'mango', 8.00);

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `uid` int NOT NULL DEFAULT 0,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `email` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `pwd` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `nick` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `desc` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `sex` int NOT NULL DEFAULT 0,
  `icon` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `uid`(`uid` ASC) USING BTREE,
  UNIQUE INDEX `email`(`email` ASC) USING BTREE,
  INDEX `name`(`name` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 745438 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES (3, 1002, 'llfc', 'secondtonone1@163.com', '654321)', 'llfc', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (4, 1003, 'tc', '18165031775@qq.com', '123456', 'tc', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (5, 1004, 'yuanweihua', '1456188862@qq.com', '}kyn;89>?<', 'yuanweihua', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (6, 1005, 'test', '2022202210033@whu.edu.cn', '}kyn;89>?<', 'test', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (8, 1007, 'fhr', '3157199927@qq.com', 'xuexi1228', 'fhr', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (9, 1008, 'zglx2008', 'zglx2008@163.com', '123456', 'zglx2008', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (13, 1012, 'resettest', '1042958553@qq.com', '230745', 'resettest', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (14, 1013, 'rss_test', '1685229455@qq.com', '123456', 'rss_test', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (15, 1014, '123456789', '1152907774@qq.com', '123456789', '123456789', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (20, 1019, 'zack', '1017234088@qq.com', '654321)', 'zack', '', 0, '34a7d7cd-dcf6-4acf-be33-cbac757c59ed.png');
INSERT INTO `user` VALUES (21, 1020, 'aatext', '1584736136@qq.com', '745230', 'aatext', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (22, 1021, 'ferrero1', '1220292901@qq.com', '1234', 'ferrero1', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (23, 1022, 'ferrero2', '15504616642@163.com', '1234', 'ferrero2', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (24, 1023, 'lyf', '3194811890@qq.com', '123456', 'lyf', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (25, 1024, 'lh', '2494350589@qq.com', 'fb8::>:;8<', 'lh', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (33, 1031, 'zjm', '1013049201@qq.com', '745230', 'zjm', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (34, 1032, 'yxc', '1003314442@qq.com', '123', 'yxc', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (37, 1035, 'wangyu', '962087148@qq.com', '123456', 'wangyu', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (39, 1037, 'chae', '318192621@qq.com', '777777', 'chae', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (40, 1038, 'summer', '1586856388@qq.com', '777777', 'summer', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (44, 1042, 'zzz', '3434321837@qq.com', '777777', '', '', 0, '');
INSERT INTO `user` VALUES (45, 1043, 'sadda', 'z1668047679@163.com', '77777', '', '', 0, '');
INSERT INTO `user` VALUES (46, 1044, 'qwe', '1368326038@qq.com', '1234', '', '', 0, '');
INSERT INTO `user` VALUES (52, 1050, 'test23', '161945471@qq.com', '230745', 'test23', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (53, 1051, '123', '1767269204@qq.com', '123', '', '', 0, '');
INSERT INTO `user` VALUES (54, 1052, 'zjc', '766741776@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (55, 1053, 'test_1', 'zzsr_0719@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (56, 1054, 'sqy', '3175614975@qq.com', '745230', 'sqy', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (57, 1055, 'ocz', 'q3175614975@163.com', '745230', 'ocz', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (58, 1056, 'test28', '1669475972@qq.com', '230745', 'test28', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (60, 1058, 'NoOne', '1764850358@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (61, 2179, 'saheun', 'jinsaheun0926@outlook.com', '654321)', 'saheun', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (62, 2180, 'csx', '2843275253@qq.com', '654321)', 'csx', '', 0, ':/res/head_6.jpg');
INSERT INTO `user` VALUES (63, 2020, 'rain', '20520@126.com', '745230', 'klaus', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (64, 2030, 'yyh', '285881076@qq.com', '745230', 'yyh', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (66, 2181, 'jmz', 'jinmz0101@outlook.com', '654321)', 'jmz', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (69, 2182, 'winter', 'winter010101@outlook.com', '654321)', 'winter', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (84, 1077, '氧白告', '3457468550@qq.com', '777777', '氧白告', '', 0, ':/res/head_21.jpg');
INSERT INTO `user` VALUES (86, 1079, 'cpt1', 'chenpaiteng@qq.com', '777777', 'cpt1', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (87, 1080, 'cpt2', 'chenpaiteng@163.com', '777777', 'cpt2', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (95, 1087, 'ame', '1161761559@qq.com', '745230', 'ame', '', 0, '');
INSERT INTO `user` VALUES (745235, 1093, 'yzmb', '11102766@qq.com', '745230', 'yzmb', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745236, 1094, 'ming', '1951831076@qq.com', '745230', 'ming', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745238, 1096, 'qqq', '1508945593@qq.com', 'wqctr', '', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745239, 1097, 'www', '2597586064@qq.com', '745230', '', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745240, 1098, 'Fany', '2848827727@qq.com', '745230', 'Fany', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745241, 1099, 'Zhang', '3228437938@qq.com', '745230', 'Zhang', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745242, 1100, '111', '3@q.com', '666666', '111', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745243, 1101, 'wdg', 'wdg@sb.com', '666666', 'wdg', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745244, 1102, 'HuYao', '3391870934@qq.com', '745230', 'HuYao', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745247, 1104, 'bishi', '3031719794@qq.com', '745745', 'bishi', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745251, 1108, 'aaa', '202201005117@email.sxu.edu.cn', '745230', '', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745252, 1109, '11', 'lv10052024@163.com', '1111', '', '', 0, '');
INSERT INTO `user` VALUES (745257, 1114, 'zay', '1979034537@qq.com', '745230', 'zay', '', 0, '');
INSERT INTO `user` VALUES (745263, 1120, 'KNO3', '3039647873@qq.com', '547547', 'KNO3', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745264, 1121, 'liushuai', '2159750441@qq.com', '745745', 'liushuai', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745265, 1122, 'Mrchang', '3482399524@qq.com', '745745', 'Mrchang', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745266, 1123, 'gordon', '1301858133@qq.com', '745230', 'gordon', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745269, 1126, 'jelly', 'gordon637@163.com', '745230', 'jelly', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745271, 1128, 'qyt', '2286765237@qq.com', '745230', 'qyt', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745272, 1129, 'maotouying', '2390742005@qq.com', '745745', 'maotouying', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745275, 1132, 'newday', '2805246514@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745276, 1133, 'haha', '18862007701@163.com', '745745', 'haha', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745277, 1134, 'gls', 'gxx@whu.edu.cn', '745745', 'gls', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745280, 1137, 'newdaytest', '15923025229@163.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745282, 1139, 'xyl', '2358078380@qq.com', '~j745', '', '', 0, '');
INSERT INTO `user` VALUES (745283, 1140, 'dst', '2141478259@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745284, 1141, 'ywttest', '19120531599@163.com', '745230', 'ywttest', '', 0, '');
INSERT INTO `user` VALUES (745285, 1142, 'wxy', '13674921583@163.com', '745230', 'wxy', '', 0, '');
INSERT INTO `user` VALUES (745286, 1143, 'poison', '270111426@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745287, 1144, 'ejnqx', '15980307945@163.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745290, 1147, '1234', 'tangjwsr@163.com', '23458', '', '', 0, '');
INSERT INTO `user` VALUES (745291, 1148, '1', '2207336992@qq.com', '11', '', '', 0, '');
INSERT INTO `user` VALUES (745292, 1149, 'mwt', '2622412201@qq.com', '123456', 'mwt', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745293, 1150, 'gsh', '2225665686@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745294, 1151, 'ffff', '2060603454@qq.com', '6543210', 'ffff', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745295, 1152, 'hxz', '2381096717@qq.com', '745230', 'hxz', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745296, 1153, 'hua', '2276128113@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745304, 1146, 'jmt', '3208064665@qq.com', '745230', '', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745305, 1154, 'jmetin', 'jmetin@163.com', '745230', '', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745308, 1165, 'glc', '3503006364@qq.com', '741852', '', '', 0, '');
INSERT INTO `user` VALUES (745310, 1167, 'glc2', '1099495783@qq.com', '147258', '', '', 0, '');
INSERT INTO `user` VALUES (745311, 1168, 'cwb', '2967229515@qq.com', '745230', 'cwb', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745312, 1169, 'cbq', '359989105@qq.com', '745230', 'cbq', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745313, 1170, 'yanyan', '1776950301@qq.com', '745230', 'yanyan', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745314, 1171, 'yjh', 'jiahao_yan@163.com', '745230', 'yjh', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745315, 1172, 'crx', '15058762422@163.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745316, 1173, 'ccrx', '2803033275@qq.com', '1234567', '', '', 0, '');
INSERT INTO `user` VALUES (745317, 1174, 'fangyee', 'fangyi0926@qq.com', '123456', 'fangyee', '', 0, '');
INSERT INTO `user` VALUES (745318, 1175, 'xiao', '2557854108@qq.com', '745230', 'xiao', '', 0, '7dfce23d-8032-4d33-af31-2b0f15ec3f63.png');
INSERT INTO `user` VALUES (745319, 1176, 'yulinyi', 'xuchen@superpiano.com.cn', '745230', 'yulinyi', '', 0, '');
INSERT INTO `user` VALUES (745321, 1178, 'llfctest', '2055181763@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745322, 1179, 'jianghao', '1464709591@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745326, 1183, '1212', '1162534442@qq.com', '747474', '1212', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745327, 1184, 'malong1', '2781164632@qq.com', 'am`cbk;>9:::', 'malong1', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745330, 1187, 'momo', '2759615633@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745331, 1188, 'lry', '1194930127@qq.com', 'gyr9;;8::9:', 'lry', '', 0, '');
INSERT INTO `user` VALUES (745332, 1189, 'smj', 'smj_0408@163.com', '745230', 'smj', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745333, 1190, 'Curl', '736605182@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745335, 1192, 'smjqq', '929713732@qq.com', 'ygb8:::<;>', 'smjqq', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745345, 1202, 'krico', '2657751462@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745348, 1205, 'z66', '1726549116@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745349, 1206, 'wxwx', '2895856612@qq.com', '123456.', '', '', 0, '');
INSERT INTO `user` VALUES (745350, 1207, 'yukang2', '2296085080@qq.com', '745230', 'yukang2', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745354, 1211, 'wwx', '18624173901@163.com', 'ibrvpp', 'wwx', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745356, 1213, 'abc123', '2460123996@qq.com', '745230', 'abc123', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745360, 1217, 'makk', 'bwmlth@foxmail.com', '745230', 'makk', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745362, 1219, 'fxw', 'fxwchengxuyuan@163.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745363, 1220, 'libai', '1977370522@qq.com', '`~q745', '', '', 0, '');
INSERT INTO `user` VALUES (745364, 1221, 'you', '1656377690@qq.com', '745230', 'you', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745365, 1222, 'zhangzhihui', 'youxw@cug.edu.cn', '745230', 'zhangzhihui', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745366, 1223, 'zsc', '3142277367@qq.com', '745230', 'zsc', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745367, 1224, '114514', '2315608252@qq.com', '114514', '', '', 0, '');
INSERT INTO `user` VALUES (745368, 1225, 'll', '1451230636@qq.com', '032547', 'll', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745369, 1226, 'xiexinyu', '872448677@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745370, 1227, 'flytest1', 'fly2908@163.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745371, 1228, 'asda', '2976159675@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745372, 1229, 'as', '1653786255@qq.com', '12345', '', '', 0, '');
INSERT INTO `user` VALUES (745373, 1230, 'lucktest', 'a2199189024@163.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745374, 1231, 'qwerty', '18714382065@163.com', '123456', 'qwerty', '', 0, '');
INSERT INTO `user` VALUES (745375, 1232, 'ajhfgsdj', '13523955317@163.com', '123456', 'ajhfgsdj', '', 0, '');
INSERT INTO `user` VALUES (745376, 1233, 'lw', '1070414135@qq.com', '1125', 'lw', '', 0, '');
INSERT INTO `user` VALUES (745377, 1234, 'wl', '1755304361@qq.com', '2234', 'wl', '', 0, '');
INSERT INTO `user` VALUES (745387, 1244, 'why', '1191416756@qq.com', '123', '', '', 0, '');
INSERT INTO `user` VALUES (745393, 1250, 'lfh', '1551868104@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745394, 1251, 'zg', '2064571645@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745398, 1255, 'hpz', '111111@qq.com', '777777', 'hpz', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745399, 1256, 'jaychou', '222222@qq.com', '444444', 'jaychou', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745400, 1257, 'sang', '111222@qq.com', '777777', 'sang', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745401, 1258, 'yan', '333444@qq.com', '777777', 'yan', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745402, 1259, 'crl', '555666@qq.com', '777777', 'crl', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745403, 1260, 'qaq', '2075605318@qq.com', '745230', 'qaq', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745404, 1261, 'qwer', '666777@qq.com', '777777', 'qwer', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745406, 1263, 'admin', '1772274756@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745407, 1264, 'admin55555', '1772274746@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745410, 1267, 'mingqianss', 'xiaoxuezha@foxmail.com', '745230', 'mingqianss', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745411, 1268, 'mingqianss2', 'mingqiantjw@163.com', '745230', 'mingqianss2', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745412, 1269, 'mingw', '3186428847@qq.com', '745230', 'mingw', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745413, 1270, 'mmmm', '13593721169@163.com', '745230', 'mmmm', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745414, 1271, 'cat', '18368122687@163.com', '745230', 'cat', '', 0, ':/res/head_3.jpg');
INSERT INTO `user` VALUES (745415, 1272, 'GuoLiJun', '15152541710@163.com', '745230', 'GuoLiJun', '', 0, ':/res/head_5.jpg');
INSERT INTO `user` VALUES (745416, 1273, 'kawa', '19072984947@163.com', '745230', 'kawa', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745417, 1274, 'xxc', '1265865927@qq.com', '745230', 'xxc', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745418, 1275, 'suliu', '2406582339@qq.com', '745230', 'suliu', '', 0, '');
INSERT INTO `user` VALUES (745419, 1276, 'dyue', '15371401089@163.com', '745230', 'dyue', '', 0, ':/res/head_4.jpg');
INSERT INTO `user` VALUES (745420, 1277, 'dqh', '3234401829@qq.com', '745230', '', '', 0, '');
INSERT INTO `user` VALUES (745421, 1278, 'xxx', '111444@qq.com', '777777', 'xxx', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745422, 1279, 'lixin', '177225@qq.com', '210', '', '', 0, '');
INSERT INTO `user` VALUES (745423, 1280, 'codexin', '321115904@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745424, 1281, 'xx', '111555@qq.com', '777777', 'xx', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745425, 1282, 'qiu', '2279039011@qq.com', '123456', '', '', 0, '');
INSERT INTO `user` VALUES (745426, 1283, 'hc', '603008751@qq.com', 'hc123', '', '', 0, '');
INSERT INTO `user` VALUES (745427, 1284, 'f', '132@qq.com', 'f', '', '', 0, '');
INSERT INTO `user` VALUES (745428, 1285, 'df', '232@qq.com', '123', '', '', 0, '');
INSERT INTO `user` VALUES (745429, 1286, 'byb', '601189025@qq.com', '123456', 'byb', '', 0, ': / res / head_1.jpg');
INSERT INTO `user` VALUES (745430, 1287, 'dyy', '1410624375@qq.com', '9:;<=>?0', '', '', 0, '');
INSERT INTO `user` VALUES (745434, 1291, 'wei', '123456@qq.com', '745230', 'wei', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745437, 1294, 'sinan', '3055460182@qq.com', '745230', '', '', 0, '');

-- ----------------------------
-- Table structure for user_id
-- ----------------------------
DROP TABLE IF EXISTS `user_id`;
CREATE TABLE `user_id`  (
  `id` int NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1295 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user_id
-- ----------------------------
INSERT INTO `user_id` VALUES (1294);

-- ----------------------------
-- Procedure structure for reg_user
-- ----------------------------
DROP PROCEDURE IF EXISTS `reg_user`;
delimiter ;;
CREATE PROCEDURE `reg_user`(IN `new_name` VARCHAR(255), 
    IN `new_email` VARCHAR(255), 
    IN `new_pwd` VARCHAR(255), 
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;

    -- 开始事务
    START TRANSACTION;

    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
        SET result = 0; -- 用户名已存在
        COMMIT;
    ELSE
        -- 用户名不存在，检查email是否已存在
        IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
            SET result = 0; -- email已存在
            COMMIT;
        ELSE
            -- email也不存在，更新user_id表
            UPDATE `user_id` SET `id` = `id` + 1;

            -- 获取更新后的id
            SELECT `id` INTO @new_id FROM `user_id`;

            -- 在user表中插入新记录
            INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (@new_id, new_name, new_email, new_pwd);
            -- 设置result为新插入的uid
            SET result = @new_id; -- 插入成功，返回新的uid
            COMMIT;

        END IF;
    END IF;

END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for reg_user_procedure
-- ----------------------------
DROP PROCEDURE IF EXISTS `reg_user_procedure`;
delimiter ;;
CREATE PROCEDURE `reg_user_procedure`(IN `new_name` VARCHAR(255), 
                IN `new_email` VARCHAR(255), 
                IN `new_pwd` VARCHAR(255), 
                OUT `result` INT)
BEGIN
                -- 如果在执行过程中遇到任何错误，则回滚事务
                DECLARE EXIT HANDLER FOR SQLEXCEPTION
                BEGIN
                    -- 回滚事务
                    ROLLBACK;
                    -- 设置返回值为-1，表示错误
                    SET result = -1;
                END;
                -- 开始事务
                START TRANSACTION;
                -- 检查用户名是否已存在
                IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
                    SET result = 0; -- 用户名已存在
                    COMMIT;
                ELSE
                    -- 用户名不存在，检查email是否已存在
                    IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
                        SET result = 0; -- email已存在
                        COMMIT;
                    ELSE
                        -- email也不存在，更新user_id表
                        UPDATE `user_id` SET `id` = `id` + 1;
                        -- 获取更新后的id
                        SELECT `id` INTO @new_id FROM `user_id`;
                        -- 在user表中插入新记录
                        INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (@new_id, new_name, new_email, new_pwd);
                        -- 设置result为新插入的uid
                        SET result = @new_id; -- 插入成功，返回新的uid
                        COMMIT;
                    END IF;
                END IF;
            END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
