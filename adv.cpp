/*
  calculator_buggy.cpp
*/

#include "std_lib_facilities.h"

struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) : kind(ch), value(0) { }
	Token(char ch, double val) : kind(ch), value(val) { }
	Token(char ch, string val) : kind(ch), name(val) { }////노출됨
};

class Token_stream {
	bool isFull;//직관적으로 수정
	Token buffer;
public:
	Token_stream() : isFull(false), buffer(0) { }//false로 수정
	Token get();
	void putback(Token t) { buffer = t; isFull = true; }
	void ignore(char);
};

const char let = 'L';//token type enum
const char mod = 'M';//modify
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';//변수토큰타입인듯

Token Token_stream::get() {
	if (isFull == true) { isFull = false; return buffer; }

	char ch;
	cin >> ch;

	switch (ch) {
	case '(': case ')':
	case '+': case '-': case '*':case '/': case '%':
	case ';': case '=':
	case '^':
		return Token(ch);

	case '.': case '0': case '1': case '2': case '3':case '4':
	case '5': case '6': case '7': case '8': case '9': {//변수명이 숫자로 시작하면 안되는이유
		cin.unget();
		double val;
		cin >> val;
		return Token(number, val);
	}

	default://left, quit,let처리
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;//한글자가 아니라 이런처리가 필요
			cin.unget();/////////

			if (s == "let") return Token(let);
			if (s == "quit") return Token(quit);
			if (s == "mod") return Token(mod);
			//let 변수 할당, 변수불러오기 둘다
			return Token(name, s);//변수 할당lhs, 불러오기 둘다rhs
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c) {
	if (isFull && c == buffer.kind) {
		isFull = false;
		return;
	}
	isFull = false;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}





class Calculator
{
public:
	Calculator();
	void calculate();
private:
	//const
	double const pi = 3.14159;//pi
	double const Yee = 2.71828;//e

	const string prompt = "> ";
	const string result = "= ";

	//let변수부분
	struct Variable {
		string name;//변수명
		double value;//값
		Variable(string n, double v) : name(n), value(v) { }
	};
	// all the remaining functions	
	double primary();
	double exponantial();
	double term();

	double expression();
	double declaration();
	
	double modi(double);
	double myDefinition();

	double statement();
	void clean_up_mess();

	double get_value(string s);
	void set_value(string s, double d);
	bool is_declared(string s);



	Token_stream ts;
	vector<Variable> names;
};


double Calculator::get_value(string s) {//찾아서 주기 //hashmap이 좋은뎅
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;//문자열비교 == 괜찮지?
	error("get: undefined name ", s);
}

void Calculator::set_value(string s, double d) {
	//<=수정
	for (int i = 0; i < names.size(); ++i)//기존거가 있으면 그걸 업데이트합니다.
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	//새거인경우 새로 추가합니다.
	names.push_back(Variable(s, d));

	//error("set: undefined name ", s);//오류는 없습니다.
}

bool Calculator::is_declared(string s) {
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}



double Calculator::primary() {
	Token t = ts.get();
	switch (t.kind) {
	case '(': {
		double d = expression();//여길 먼저계산함 괄호안
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
		return d;
	}
	case '-'://구현되있는거 아닌가?
		return -primary();

	case '+':  //+unary
		return primary();

	case number:
		// cout << "pri" << t.value;
		return t.value;

	case name://변수토큰 rhs
		if (!is_declared(t.name))
			error("error");
		return get_value(t.name);

	default:
		error("primary expected");
	}
}

double Calculator::exponantial() {
	vector<double> exp;
	exp.push_back(primary());//dobule 오류처리

	while (true) {
		Token t = ts.get();

		switch (t.kind) {
		case '^':
			exp.push_back(primary());//우측부터 연산하므로 모아서 나중에(연쇄가 끝날때) 다 연산한다
			break;
		default://///노출 여기 조심
			ts.putback(t);
			double right = 1;//곱의 항등원
			do {
				double base = exp.back(); exp.pop_back();
				right = pow(base, right);
			} while (!exp.empty());//다 처리한경우 아웃!
			return right;
		}
	}
}

double Calculator::term() {
	// cout << "Term";
	double left = exponantial();
	while (true) {
		Token t = ts.get();
		// cout << "Termin while";

		switch (t.kind) {
		case '*':
			// cout << "*";
			left *= exponantial();
			break;

		case '/': {
			// cout << "/";
			double d = exponantial();
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		default:
			// cout << "term" << left;
			ts.putback(t);
			return left;
		}
	}
}


double Calculator::expression() {
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.putback(t);
			// cout << "exp" << left;
			return left;
		}
	}
}

//선언쪽
double Calculator::declaration() {//let 빨고 난후
	Token t = ts.get();//rhs임

	//오류처리
	if (t.kind != name) error("name expected in declaration");//변수명이 안옴 ㅠ
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice");//중복선언
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);//=없는경우

	double d = expression();//뒤의 연산
	set_value(name, d);//set함수는 원래 업데이트도 되지만 중복선언을 위에서 체크하니 사실상 새변수추가기능
	//갱신은 없으니까 아래 코드도 동일작동한다
	//names.push_back(Variable(name, d));
	return d;
}



double Calculator::modi(double oldval) {
	Token t = ts.get();
	switch (t.kind) {
	case '='://변수 갱신
		return expression();
	case '+': {
		Token t2 = ts.get();
		switch (t2.kind) {
		case '=':
			return oldval + expression();
		case '+':
			return ++oldval;
		default:
			//에러
			error("Syntax error");
		}
	}
	case '-': {
		Token t2 = ts.get();
		switch (t2.kind) {
		case '=':
			return oldval - expression();
		case '-':
			return --oldval;
		default:
			//에러		
			error("Syntax error");
		}
	}
	case '*': {
		Token t2 = ts.get();
		switch (t2.kind) {
		case '=':
			return oldval * expression();
		case '*':
			return oldval * oldval;//^2
		default:
			//에러
			error("Syntax error");
		}
	}
	case '/': {
		Token t2 = ts.get();
		switch (t2.kind) {
		case '=':
			return oldval / expression();
		default:
			//에러
			error("Syntax error");
		}
	}
	default:
		error("Syntax error");
	}
}

double Calculator::myDefinition() {
	Token t = ts.get();	//lhs
	double oldval = get_value(t.name);
	double newval = modi(oldval);
	set_value(t.name, newval);
	return newval;
}

double Calculator::statement() {//첫타에서 3가지로 가른다
	Token t = ts.get();
	switch (t.kind) {
	case let://변수선언
		return declaration();
	case mod: //변수 갱신 작업
		return myDefinition();			
	default:
		ts.putback(t);
		return expression();
	}
}


void Calculator::clean_up_mess() {
	ts.ignore(print);
}

Calculator::Calculator() {
	set_value("pi", pi);/////right?
	set_value("e", Yee);
}

void Calculator::calculate() {
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();//;전부 빨기
		if (t.kind == quit) return;
		ts.putback(t);

		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()
try {
	Calculator c;
	c.calculate();

	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	keep_window_open();
	return 1;
}
catch (...) {
	cerr << "exception\n";
	keep_window_open();
	return 2;
}
