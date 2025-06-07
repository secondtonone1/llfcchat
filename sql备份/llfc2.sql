/*
 Navicat Premium Data Transfer

 Source Server         : llfcmysql
 Source Server Type    : MySQL
 Source Server Version : 80027 (8.0.27)
 Source Host           : 81.68.86.146:3308
 Source Schema         : llfc

 Target Server Type    : MySQL
 Target Server Version : 80027 (8.0.27)
 File Encoding         : 65001

 Date: 04/06/2025 10:05:59
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

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
) ENGINE = InnoDB AUTO_INCREMENT = 185 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

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
INSERT INTO `friend` VALUES (183, 1002, 1019, 'zack');
INSERT INTO `friend` VALUES (184, 1019, 1002, '');

-- ----------------------------
-- Table structure for friend_apply
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply`;
CREATE TABLE `friend_apply`  (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `from_uid` int NOT NULL,
  `to_uid` int NOT NULL,
  `status` smallint NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 144 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of friend_apply
-- ----------------------------
INSERT INTO `friend_apply` VALUES (49, 1054, 1055, 1);
INSERT INTO `friend_apply` VALUES (52, 1056, 1012, 0);
INSERT INTO `friend_apply` VALUES (64, 1032, 1035, 0);
INSERT INTO `friend_apply` VALUES (68, 1037, 1038, 1);
INSERT INTO `friend_apply` VALUES (99, 2030, 2020, 1);
INSERT INTO `friend_apply` VALUES (111, 1080, 1079, 1);
INSERT INTO `friend_apply` VALUES (123, 2181, 2182, 1);
INSERT INTO `friend_apply` VALUES (125, 1079, 1080, 1);
INSERT INTO `friend_apply` VALUES (126, 1090, 2020, 1);
INSERT INTO `friend_apply` VALUES (127, 2030, 1090, 1);
INSERT INTO `friend_apply` VALUES (128, 2020, 1090, 0);
INSERT INTO `friend_apply` VALUES (129, 1093, 2020, 1);
INSERT INTO `friend_apply` VALUES (130, 2030, 1093, 1);
INSERT INTO `friend_apply` VALUES (132, 1080, 1085, 1);
INSERT INTO `friend_apply` VALUES (134, 1079, 1085, 1);
INSERT INTO `friend_apply` VALUES (135, 1094, 1087, 1);
INSERT INTO `friend_apply` VALUES (141, 1002, 1042, 0);
INSERT INTO `friend_apply` VALUES (142, 1002, 1008, 0);
INSERT INTO `friend_apply` VALUES (143, 1019, 1002, 1);

-- ----------------------------
-- Table structure for group_chat
-- ----------------------------
DROP TABLE IF EXISTS `group_chat`;
CREATE TABLE `group_chat`  (
  `thread_id` bigint UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '群聊名称',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`thread_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of group_chat
-- ----------------------------

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

-- ----------------------------
-- Table structure for private_chat
-- ----------------------------
DROP TABLE IF EXISTS `private_chat`;
CREATE TABLE `private_chat`  (
  `thread_id` bigint UNSIGNED NOT NULL AUTO_INCREMENT,
  `user1_id` bigint UNSIGNED NOT NULL,
  `user2_id` bigint UNSIGNED NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`thread_id`) USING BTREE,
  UNIQUE INDEX `uniq_private_thread`(`user1_id` ASC, `user2_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of private_chat
-- ----------------------------

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
) ENGINE = InnoDB AUTO_INCREMENT = 745238 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES (3, 1002, 'llfc', 'secondtonone1@163.com', '654321)', 'llfc', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (4, 1003, 'tc', '18165031775@qq.com', '123456', 'tc', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (5, 1004, 'yuanweihua', '1456188862@qq.com', '}kyn;89>?<', 'yuanweihua', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (6, 1005, 'test', '2022202210033@whu.edu.cn', '}kyn;89>?<', 'test', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (8, 1007, 'fhr', '3157199927@qq.com', 'xuexi1228', 'fhr', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (9, 1008, 'zglx2008', 'zglx2008@163.com', '123456', 'zglx2008', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (13, 1012, 'resettest', '1042958553@qq.com', '230745', 'resettest', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (14, 1013, 'rss_test', '1685229455@qq.com', '123456', 'rss_test', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (15, 1014, '123456789', '1152907774@qq.com', '123456789', '123456789', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (20, 1019, 'zack', '1017234088@qq.com', '654321)', 'zack', '', 0, ':/res/head_1.jpg');
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
INSERT INTO `user` VALUES (45, 1043, 'sadda', 'z1668047679@163.com', '777777', '', '', 0, '');
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
INSERT INTO `user` VALUES (93, 1085, 'xiao', '2557854108@qq.com', '745230', 'xiao', '', 0, '');
INSERT INTO `user` VALUES (95, 1087, 'ame', '1161761559@qq.com', '745230', 'ame', '', 0, '');
INSERT INTO `user` VALUES (745235, 1093, 'yzmb', '11102766@qq.com', '745230', 'yzmb', '', 0, ':/res/head_1.jpg');
INSERT INTO `user` VALUES (745236, 1094, 'ming', '1951831076@qq.com', '745230', 'ming', '', 0, ':/res/head_2.jpg');
INSERT INTO `user` VALUES (745237, 1095, '2333334', '3328713312@qq.com', '12345', '2333334', '', 0, '479b');

-- ----------------------------
-- Table structure for user_id
-- ----------------------------
DROP TABLE IF EXISTS `user_id`;
CREATE TABLE `user_id`  (
  `id` int NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user_id
-- ----------------------------
INSERT INTO `user_id` VALUES (1095);

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
