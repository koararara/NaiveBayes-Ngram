#pragma once

#include <unordered_map>
#include <string>
#include <numeric>	//	accumulate
#include <cmath>	//	log
#include <cfloat>	//	DBL_MAX

namespace nb {
	using namespace std;

	using Count = unordered_map<wstring, int>;
	using CountWords = unordered_map<wstring, Count>;

	/*-------------------------------------------------------------------------
	 *	ナイーブベイジアンフィルタ
	 *	単語出現頻度を統計処理することで文書のカテゴリ分類を行う
	 *	単語の切り分けは、言語知識を持ちいらないNグラム分割によって行う
	 *	ただし、形態素解析などによる分割処理を行いたい場合は、派生クラスを
	 *	作成し、parseWordsメンバ関数を差し替えることで実行できる
	 *-------------------------------------------------------------------------*/
	class NaiveBayes
	{
		NaiveBayes(const NaiveBayes& other) = delete;
		NaiveBayes& operator=(const NaiveBayes& other) = delete;

	protected:
		int gram_ = 2;						//	N-Gramの"N"(分割数)
		Count			category_;			//	カテゴリと(訓練中の)登場回数
		CountWords		words_;				//	単語(カテゴリ毎の単語の登場回数)
		Count			vocabularies_;		//	語彙(カテゴリ関係なしの全体の語彙)

		/*---------------------------------------------------------------------
		 *	文書中から単語を切り出す処理
		 *	doc    : 文章
		 *	戻り値 : 単語ごとの出現回数
		 *---------------------------------------------------------------------
		 *	メモ
		 *	英文であれば、スペースによる分かち書きをしているため、
		 *	それにより単語切り出しが可能だが、日本語の場合は、形態素解析が
		 *	必要で、面倒。
		 *	ナイーブベイズアルゴリズムは単語の出現場所による共起を考慮しないで
		 *	独立性を仮定したアルゴリズムなので、Nグラムアルゴリズムによって分割
		 *	した単語でも同様に期待できる
		 *---------------------------------------------------------------------*/
		virtual Count SplitWords(const wstring& doc) {
			Count ret;
			for (size_t i = 0; i + gram_ <= doc.length(); i++) {
				ret[doc.substr(i, gram_)]++;
			}
			return ret;
		}

		/*---------------------------------------------------------------------
		 *	カテゴリに属している単語数を得る
		 *	cate   : カテゴリ
		 *	戻り値 : 単語数
		 *---------------------------------------------------------------------*/
		int SumCategory(const wstring& cate) {
			return accumulate(cbegin(words_[cate]), cend(words_[cate]), 0, [](int sum, auto& w) { return sum + w.second; });
		}

		/*---------------------------------------------------------------------
		 *	カテゴリに単語(word)が出現した回数を返す
		 *	word   : 単語
		 *	cate   : カテゴリ
		 *	戻り値 : 出現回数
		 *---------------------------------------------------------------------*/
		int InCategory(const wstring& word, const wstring& cate) {
			return (words_[cate])[word];
		}

		/*---------------------------------------------------------------------
		 *	与えられた単語群が指定カテゴリであると思われる点数を返す
		 *	words    : 文章に属する単語群
		 *	cate     : カテゴリ
		 *	sum_cate : カテゴリに属している単語数
		 *	戻り値   : 得点
		 *---------------------------------------------------------------------*/
		double Score(const unordered_map<wstring, int>& words, const wstring& cate, int sum_cate) {
			//	対数(log)を取っているのは、アンダーフロー対策(膨大な語彙に極僅かな単語の件数を計算するためアンダーフローが発生しやすい)
			double s = log((double)category_[cate] / category_.size());

			for (auto& w : words) {
				//	+1.0はゼロ頻度問題対策の一つラプラス法を用いている為の値
				s += log((InCategory(w.first, cate) + 1.0) / (double)(sum_cate + vocabularies_.size()*1.0));
			}
			return s;
		}
	public:
		NaiveBayes(int n) : gram_(n) {}
		virtual ~NaiveBayes() {}

		/*---------------------------------------------------------------------
		 *	訓練
		 *	doc  : 文章
		 *	cate : カテゴリ
		 *---------------------------------------------------------------------*/
		void Training(const wstring& doc, const wstring& cate) {
			auto words = SplitWords(doc);
			for (auto& w : words) {
				vocabularies_[w.first] = 0;				//	語彙(単語を登録)
				(words_[cate])[w.first] += w.second;	//	カテゴリに属する単語の出現回数を数える
			}
			category_[cate]++;
		}

		/*---------------------------------------------------------------------
		 *	分類
		 *	doc    : カテゴリ不明な文書
		 *	戻り値 : 推定したカテゴリ名
		 *---------------------------------------------------------------------*/
		wstring Classifier(const wstring& doc) {
			wstring best;
			double max = -DBL_MAX;
			auto words = SplitWords(doc);

			for (auto& cate : category_) {
				double s = Score(words, cate.first, SumCategory(cate.first));
				if (s > max) {
					max = s;
					best = cate.first;
				}
			}

			return best;
		}

	};
}
