#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
  Email(string from_, string to_, string body_) {
	  from = from_;
	  to = to_;
	  body = body_;
  }
  string from;
  string to;
  string body;
};


class Worker {
public:
  virtual ~Worker() = default;
  virtual void Process(unique_ptr<Email> email) {
	  if(next_) {
		  next_->Process(std::move(email));
	  }
  }
  virtual void Run() {
    throw logic_error("Unimplemented");
  }

protected:

  unique_ptr<Worker> next_;

public:
  void SetNext(unique_ptr<Worker> next) {
	  next_ = move(next);
  }
};


class Reader : public Worker {
private:
	istream& input_;
public:
    Reader(istream& input): input_(input) {}
    void Run() override {
    	string from, to, body;
    	while(getline(input_, from)) {
    		getline(input_, to);
    		getline(input_, body);
    		unique_ptr<Email> email = make_unique<Email>(from, to, body);
    		Worker::Process(std::move(email));
    	}
    }

    void Process(unique_ptr<Email> email) override {
    	Worker::Process(std::move(email));
    }
};


class Filter : public Worker {
public:
  using Function = function<bool(const Email&)>;
  Filter(Function func): func_(func) {}
  void Process(unique_ptr<Email> email) override {
	  if(func_(*email)) {
		  Worker::Process(move(email));
	  }
  }
private:
  Function func_;
};


class Copier : public Worker {
private:
	string to_;
public:
  Copier(string& to):to_(to) {}
  void Process(unique_ptr<Email> email) override {
	  string from = email->from;
	  string body = email->body;
	  string to = email->to;
	  Worker::Process(std::move(email));
	  if(to_ != to) {
		  Worker::Process(make_unique<Email>(from, to_, body));
	  }
  }
};


class Sender : public Worker {
private:
	ostream& out_;
public:
  Sender(ostream& out): out_(out) {}
  void Process(unique_ptr<Email> email) override {
	  out_ << email->from << '\n';
	  out_ << email->to << '\n';
	  out_ << email->body << '\n';
  }
};


// реализуйте класс
class PipelineBuilder {
private:
	vector<unique_ptr<Worker>> workers;
public:
  // добавляет в качестве первого обработчика Reader
  explicit PipelineBuilder(istream& in) {
	  workers.push_back(make_unique<Reader>(in));
  }

  // добавляет новый обработчик Filter
  PipelineBuilder& FilterBy(Filter::Function filter) {
	  workers.push_back(make_unique<Filter>(filter));
	  return *this;
  }

  // добавляет новый обработчик Copier
  PipelineBuilder& CopyTo(string recipient) {
	  workers.push_back(make_unique<Copier>(recipient));
	  return *this;
  }

  // добавляет новый обработчик Sender
  PipelineBuilder& Send(ostream& out) {
	  workers.push_back(make_unique<Sender>(out));
	  return *this;
  }

  // возвращает готовую цепочку обработчиков
  unique_ptr<Worker> Build() {
	  for(auto it = rbegin(workers); it != prev(rend(workers)); ++it) {
		  (*next(it))->SetNext(std::move(*it));
	  }
	  return move(*begin(workers));
  }
};


void TestSanity() {
  string input = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "ralph@example.com\n"
    "erich@example.com\n"
    "I do not make mistakes of that kind\n"
  );
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy([](const Email& email) {
    return email.from == "erich@example.com";
  });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "erich@example.com\n"
    "richard@example.com\n"
    "Are you sure you pressed the right button?\n"
  );

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSanity);
  return 0;
}
