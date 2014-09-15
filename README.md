LiveRefresh
===========

A live coding command line tool for cocos2d-js / cocos2d-x lua binding!

This tool do the same thing of [Cocos Code IDE](http://www.cocos2d-x.org/products/codeide) Live Coding feature.

> No more waiting for the game to recompile or re-launch, now you are able to make live modifications to the game and see the real time results.

With this tools you can enjoy this feature freely with any code editor!!!

## Require

1. Mac OS platform
2. cocos2d-js 3.2 runtime or cocos2d-x lua-binding runtime(I didn't test it).

## Usage

1. Build this project in XCode, it will generate a executable binary.
2. Launch a runtime Cococs2d App. （Runtime feature take effect under debug build by default.）
3. Run this binary in terminal with correct args, for example:

		LiveRefresh 127.0.0.1 /your/project/root/folder
	
4. Then, you can use Cocos Console command, such as `fps off`、`version` or your self custom commands.

5. Do some modifications with your code or resources, then `refresh` command will check the diff between your project foler on your mac and your app r/w folder on your phone/simulator, and update files then reload. So, you could update your code lively! Enjoy it!!

## Implementation

This just some implementation tips, you can read the source code to check it out.

1. Socket connect cocos console port, send/rev console command.
2. Socket connect FileServer which defined in `Runtime.cpp`, send/write file contents to app.
3. Use `sendrequest` custom command defined in `Runtime.cpp` read `fileinfo_debug.json` to check diff.

## 为中文用户

专为不折腾不舒服斯基们打造，快快试用吧，很cool的！ʅ(‾◡◝)
