# NaiveBayes × N-gram 分類検証

## 概要

本プロジェクトは、**ナイーブベイズ分類器に N-gram を組み合わせることで、分類精度と言語非依存性の検証を行うこと** を目的としています。  
一般的な形態素解析を用いた前処理とは異なり、**N-gram を活用することで特定の言語に依存せず、汎用的なテキスト分類が可能** であると予想されます。  
この手法の有効性を実証するため、**デモプログラムを開発し、実際のデータを用いた評価を行います**。

## 構成

```
/NaiveBayes-Ngram/
├── src/ # ソースコード
│ ├── main_demo.cpp # デモ版
│ ├── main_data.cpp # データファイル読み込み版
│ ├── NaiveBayes.hpp # 共通ヘッダファイル
├── datafiles/ # 言語データファイル
│ ├── arabic.csv # アラビア語（アプリ用設定）
│ ├── arabic.txt # 学習材料
│ ├── arabic_2.txt # 分類検証用テキスト
│ ├── chinese.csv # 中国語（繫体字）
│ ├── chinese.txt
│ ├── chinese_2.txt
│ ├── english.csv # 英語
│ ├── english.txt
│ ├── english_2.txt
│ ├── french.csv # フランス語
│ ├── french.txt
│ ├── french_2.txt
│ ├── japanese.csv # 日本語
│ ├── japanese.txt
│ ├── japanese_2.txt
│ ├── korean.csv # 韓国語
│ ├── korean.txt
│ ├── korean_2.txt
├── Makefile # nmake(MSVC)ビルド用
├── GNUMakefile # make(g++/clang++)ビルド用
```

## 必要環境

- **C++17**（本プロジェクトは C++17 に準拠）
- Windows: `nmake`
- Linux: `GNU make`
- Visual Studio / g++ / clang++で確認済み

## インストール方法

### Windows (MSVC)

```
nmake demo  # デモ版をビルド
nmake data  # データ処理版をビルド
```

### Linux

```
make demo  # デモ版をビルド
make data  # データ処理版をビルド
make CC=clang++ demo # コンパイラをclang++を使用してのビルド
```

## 使い方

### デモ版の実行

デモ版は、固定のサンプルデータを使用して分類を実行します。

#### Windows

```
demo.exe
```

#### Linux

```
./demo
```

### データファイル版の実行

データファイル版は、外部データを読み込み、N-gram 分割を調整しながら分類を行うことが可能です。

#### Windows

```
data.exe datafile/japanese.csv
data.exe datafile/french.csv 4 # N-gramの分割数を4に設定
```

#### Linux

```
./data datafiles/japanese.csv
./data datafiles/french.csv 4 # N-gramの分割数を4に設定
```

### 補足

- データファイルは `datafiles/`に格納されています。
- データファイルは`.csv`ファイルがプログラムが認識できるファイルです
- N-gram の分割数 を変更することで、分類の粒度を調整できます（デフォルト値は 2）。

## ライセンス

MIT License

## 作者情報

このプロジェクトは、ナイーブベイズ分類器のデモ用として作成されました。  
詳細については、README の説明やコメントをご参照ください。
