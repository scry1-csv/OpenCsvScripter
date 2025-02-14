# OpenCsvScripter

Vorze A10サイクロンSA / UFOSA / UFOTW 用のCSVスクリプトファイルの編集ソフトです。

[OpenFunscripter](https://github.com/OpenFunscripter/OFS)をフォークして作られています。

OpenGL, SDL2, ImGui, libmpv, その他多くの偉大な [ライブラリ](https://github.com/OpenFunscripter/OpenFunscripter/tree/master/lib) に依存しています。

## ダウンロード

[ここ](https://github.com/scry1-csv/OpenCsvScripter/releases/)からダウンロードできます。

## OpenFunscripterからの変更点

- スクリプトタイムラインのグラフ表示を回転型スクリプトに向いた矩形波に
- Vorze A10サイクロンSA / UFOSA / UFOTW 用のCSVファイルをエクスポート可能に

## UFOTW用CSVのエクスポートについて

プロジェクトにスクリプトを2つ以上読み込んでいる場合、メニューの「File」→「Export...」→「Export as UFOTW script」でエクスポートできます。

上から1つ目のスクリプトをUFOTWの左側、2つ目のスクリプトを右側として保存します。

現在のところ、3つ以上読み込んでいる中から選択したり左右を入れ替えてエクスポートする機能はありません。

## 日本語版について

[OG9さん制作のOpenFunscripter日本語化ファイル](https://ci-en.dlsite.com/creator/17620/article/810051#e3a487a527)を、許可を得た上でいくつかの変更を加えて同梱しています。ここに御礼を申し上げます。

## ビルド方法

1. リポジトリをクローン
2. `cd "OpenCsvScripter"`
3. `git submodule update --init`
4. CMakeを実行してコンパイル

## 利用しているWindows向けlibmpvバイナリ

Currently using: [mpv-dev-x86_64-v3-20220925-git-56e24d5.7z (it's part of the repository)](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)

## 対応プラットフォーム

現在のところ、Windows向けのバイナリのみ提供しています。
LinuxやOSXでも理論上はビルド可能のはずですが、当方では確認していません。

## バージョン履歴

### 2025/02/14 - v0.0.2
- 設定ディレクトリがOpenFunscripterと共通だったのをOpenCsvScripter専用ディレクトリに修正
- 日本語化ファイルをデフォルトで同梱

### 2025/02/09 - v0.0.1
-最初のリリース

## ライセンス
GPLv3