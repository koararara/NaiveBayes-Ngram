#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <locale>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include "NaiveBayes.hpp"

#ifdef _MSC_VER
#include <windows.h>
#define MBSTOWCS(dest, src, n) MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, n)
#else
#define MBSTOWCS(dest, src, n) mbstowcs(dest, src, n)
#endif

using namespace std;

/*---------------------------------------------------------------------
 *	ファイルを行毎に引数のラムダを使って処理を行う関数
 *	filename : ファイル名
 *	func     : ラムダ
 *	戻り値    : Container型のオブジェクトに格納して返す。Containerの型は呼び出し元が
 *	           指定できるため、任意のコンテナが利用可能
 *---------------------------------------------------------------------*/
template <typename Container, typename Func>
Container process_lines(const string& filename, Func func) {
	ifstream stream(filename, ios::binary);
	if (!stream) {
		throw runtime_error("Failed to open file: `" + filename + "`");
	}

	Container result;
	string line;

	while (getline(stream, line)) {
		line.erase(remove(line.begin(), line.end(), '\r'), line.end()); // 改行削除
		func(result, line); // ラムダ関数を適用
	}

	return result;
}

/*---------------------------------------------------------------------
 *	utf8をutf16に変換する
 *	utf8        : utf8の文字列
 *	locale_name : utf-8からutf-16に変換するためのファイルのロケールを指定する
 *	戻り値       : 変換した値(utf-16)
 *---------------------------------------------------------------------*/
std::wstring utf8_to_utf16(const std::string& utf8, const string& locale_name = "") {
	setlocale(LC_ALL, locale_name.c_str());
	
	size_t len = MBSTOWCS(nullptr, utf8.c_str(), 0);
	if (len == static_cast<size_t>(-1)) {
		throw std::runtime_error("Conversion failed!");
	}
	std::vector<wchar_t> wstr(len + 1);
	MBSTOWCS(wstr.data(), utf8.c_str(), len + 1);

	return std::wstring(wstr.data());
}

/*---------------------------------------------------------------------
 *	vector<string>をvector<wstring>に変換する
 *	input       : 変換元のデータ
 *	locale_name : utf-8からutf-16に変換するためのファイルのロケールを指定する
 *	戻り値       : 変換した値
 *---------------------------------------------------------------------*/
vector<wstring> convert_vector(const vector<string>& input, const string& locale_name) {
	vector<wstring> output(input.size());

	transform(input.begin(), input.end(), output.begin(), [&locale_name](const string& s) {
		return utf8_to_utf16(s, locale_name);
	});
	return output;
}

/*---------------------------------------------------------------------
 *	vector<vector<string>>をpair<int, wstring>に変換する
 *	「カテゴリ番号\t検証用の文」というファイルを読み込んでいるデータをvector<pair<int, wstring>>
 *	の扱いやすい形に変換する(CSVファイルをパース処理してvector<vector<>>の形になっている)。
 *	data        : {{"カテゴリ番号", "テキスト"}, {...}}の変換元のデータ
 *	locale_name : utf-8からutf-16に変換するためのファイルのロケールを指定する
 *	戻り値       : 変換した値
 *---------------------------------------------------------------------*/
vector<pair<int, wstring>> convert_pairs(const vector<vector<string>>& data, const string& locale_name) {
	vector<pair<int, wstring>> result;

	for (const auto& row : data) {
		if (row.size() < 2) continue; // 不正な行をスキップ
		result.emplace_back(stoi(row[0]), utf8_to_utf16(row[1], locale_name));
	}
	return result;
}

/*---------------------------------------------------------------------
 *	学習材料ファイルを読み込む
 *	filename    : 学習材料のファイル名リスト
 *	categories  : 分類（学習材料ファイルは各行が分類のインデックスになるように保存している)
 *	locale_name : utf-8からutf-16に変換するためのファイルのロケールを指定する
 *	戻り値       : カテゴリをキーとし、値が学習材料文のunordered_mapを返す。
 *	              (mapは内部でソートを行うため、ロケールの影響を受ける可能性(最悪停止する)
 *	              があるのでunordered_mapを使う)
 *---------------------------------------------------------------------*/
unordered_map<wstring, wstring> load_training_files(const vector<string>& filenames, 
	const vector<wstring>& categories, const string& locale_name) {
	unordered_map<wstring, wstring> result;

	for_each(filenames.cbegin(), filenames.cend(), [&](const string& filename) {
		size_t index = 0;
		auto temp = process_lines<unordered_map<wstring, wstring>>(filename, 
			[&](auto& container, const string& line) {
				if (index < categories.size()) {
					container[categories[index++]] += utf8_to_utf16(line, locale_name); // 行を連結
				}
			}
		);
		// 既存データに追加
		for (const auto& [key, value] : temp) {
			result[key] += value;
		}
	});
	return result;
}

/*---------------------------------------------------------------------
 *	アプリケーション設定ファイルを読み込む
 *	filename    : アプリケーションの指定フォーマットのCSVファイル
 *	戻り値       : 匿名構造体でベイジアンフィルターの動作検証に必要なデータ詰めて返す
 *---------------------------------------------------------------------*/
auto loadCsv(const string& filename) {
	using Csv = vector<vector<string>>;
	
	// process_linesに渡す、CSV用のラムダ関数
	auto csv_process = [](Csv& container, const string& line) {
		vector<string> row;
		stringstream ss(line);
		string cell;

		while (getline(ss, cell, '\t')) {
			row.push_back(cell);
		}
		container.push_back(row);
	};
	
	//	.csvファイルの読み込み
	auto csv_data = process_lines<Csv>(filename, csv_process);
	//	フォーマット確認(簡易的に行数のみのチェック)
	if (csv_data.size() != static_cast<size_t>(4)) {
		throw runtime_error("CSV format error");
	}
	
	//	設定ファイル(.csv)から読み込む学習材料などのファイル名にパスを追加する
	filesystem::path filepath(filename);
	filesystem::path parent_dir = filepath.parent_path();
	csv_data[3][0] = (parent_dir / csv_data[3][0]).string();
	for (std::string& name : csv_data[2]) {
		name = (parent_dir / name).string();  // `filenames` を直接更新
	}

	const string& locale_name = csv_data[0][0];	// ロケール取得
	const auto categories = convert_vector(csv_data[1], locale_name);	// 分類カテゴリを取得
	auto validate_data = process_lines<Csv>(csv_data[3][0], csv_process);	// 検証用文ファイルを読み込む
	
	struct {
		string locale_name;
		vector<wstring> category;
		unordered_map<wstring, wstring> training_data;
		vector<pair<int, wstring>> test_data;
	} data = {
		locale_name, 
		categories,
		load_training_files(csv_data[2], categories, locale_name),
		convert_pairs(Csv(validate_data.cbegin(), validate_data.cend()), locale_name) };
	return data;
}

/*---------------------------------------------------------------------
 *	使い方の表示
 *---------------------------------------------------------------------*/
void show_usage() {
	wcout << L"Usage: program_name <csv_file> [n-gram_size]" << endl << endl;
	wcout << L"Arguments:" << endl;
	wcout << L"  <csv_file>    Path to the CSV file to process." << endl;
	wcout << L"  [n-gram_size] Optional. Number of words per n-gram (default: 2)." << endl;
}

/*---------------------------------------------------------------------
 *	メイン
 *	使い方
 *	プログラム名 csvファイル [n-gramの分割数]
 *---------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	try {
#ifdef _MSC_VER
		// 出力コードページを UTF-8 (65001) に変更
		SetConsoleOutputCP(65001);
#endif
		if (argc==1 || argc >= 4) {
			show_usage();
			return 2;
		}
		std::vector<std::string> args(argv+1, argv + argc); // `std::vector<std::string>` に変換
		auto [locale_name, categories, training, test_data ] = loadCsv(args[0]);
		int n_gram = args.size() > 1 ? stoi(args[1]) : 2;
		
		// **グローバルロケールを設定**
		setlocale(LC_ALL, locale_name.c_str());

		// **標準出力用のデフォルトロケール**
		wcout.imbue(locale(locale_name.c_str()));
		
		nb::NaiveBayes b(n_gram);
		
		//	文書のカテゴリ分類のトレーニングを行う
		for (auto& category : categories) {
			if (training.find(category) != training.end()) {
				b.Training(training[category], category);
			}
		}
		int count = 0;
		for (auto& [index, validate] : test_data) {
			auto answer = b.Classifier(validate);
			wcout << std::left << std::setw(30) << validate << L"\t=> Response: " << std::setw(6) << answer;
			wcout << L" (Correct: " << categories[index] << L")";
			if (answer != categories[index]) {
				wcout << "*";
				++count;
			}
			wcout << endl;
		}
		wcout << endl;
		if (count) {
			wcout << L"\t" << count << L" errors" << endl;
		} else {
			wcout << L"\tAll correct" << endl;
		}
	} catch (const exception& e) {
		wcerr << L"Error: " << utf8_to_utf16(e.what()) << endl;
		show_usage();
		return 1;
	}
	return 0;
}
