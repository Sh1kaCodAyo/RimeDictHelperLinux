#!/bin/bash

# 1. 退出码初始化
EXIT_CODE=0

# 2. 触发 Fcitx5 重新部署 Rime
# 这里的 fcitx5-remote -r 等同于 Windows 下的 WeaselDeployer.exe /deploy
#fcitx5-remote -r
#rime_deployer --build ~/.local/share/fcitx5/rime
# 先通知 Fcitx5 退出
fcitx5-remote -e
# 然后重新启动它（让它自己触发一次完整加载）
setsid fcitx5 -d >/dev/null 2>&1 < /dev/null &
if [ $? -ne 0 ]; then
    echo "Rime 重新部署失败！请检查 Fcitx5 是否正在运行。"
    EXIT_CODE=1
fi

## 3. 执行 Git 同步操作
## 假设当前脚本就在 Rime 目录下执行，或者你可以在 cd 到工作目录
#cd "$(dirname "$0")" # 确保在当前目录执行
#
#git add .
## 检查是否有变化需要 commit
#if ! git diff-index --quiet HEAD --; then
#    git commit -m "Auto sync from RimeDictHelper: $(date '+%Y-%m-%d %H:%M:%S')"
#    git push
#    if [ $? -ne 0 ]; then
#        echo "Git 推送失败，请检查网络或冲突。"
#        EXIT_CODE=1
#    fi
#else
#    echo "没有文件变化，跳过 Git 提交。"
#fi
# 3. Git 操作
git add .
if ! git diff-index --quiet HEAD --; then
    # 在测试分支上，commit message 带个标识
    git commit -m "TEST: Auto sync from C++ tool $(date '+%H:%M:%S')"

    # 【最关键的一行】测试分支直接拦截 push！
    # 如果你真的想推到远端测试分支，可以改成 git push origin test-workspace
    echo "【测试模式】本地已 Commit，但为了安全阻止了 Push。"
    # git push
fi

exit $EXIT_CODE