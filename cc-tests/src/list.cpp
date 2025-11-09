#include "cc/log.hpp"
#include "cc/test.hpp"
#include "cc/list.hpp"
#include "cc/fmt.hpp"


mTestCase(list_default_ctor) {
  List<int> l;
  mRequire(l.empty());
  mRequire(l.size() == 0);
  mRequire(l.end() == l.begin());
}


mTestCase(list_push_pop_back) {
  List<int> l;
  l.push_back(1);
  l.push_back(2);
  l.push_back(3);

  mRequire(l.size() == 3);
  mRequire(!l.empty());
  auto it = l.begin();
  mRequire(*it == 1);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 3);
  ++it;
  mRequire(it == l.end());

  l.pop_back();

  mRequire(l.size() == 2);
  mRequire(!l.empty());
  it = l.begin();
  mRequire(*it == 1);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(it == l.end());

  l.pop_back();

  mRequire(l.size() == 1);
  mRequire(!l.empty());
  it = l.begin();
  mRequire(*it == 1);
  ++it;
  mRequire(it == l.end());

  l.pop_back();

  mRequire(l.size() == 0);
  mRequire(l.empty());
  it = l.begin();
  mRequire(it == l.end());
}


mTestCase(list_push_pop_front) {
  List<int> l;
  l.push_front(1);
  l.push_front(2);
  l.push_front(3);

  mRequire(l.size() == 3);
  mRequire(!l.empty());
  auto it = l.begin();
  mRequire(*it == 3);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 1);
  ++it;
  mRequire(it == l.end());

  l.pop_front();

  mRequire(l.size() == 2);
  mRequire(!l.empty());
  it = l.begin();
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 1);
  ++it;
  mRequire(it == l.end());

  l.pop_front();

  mRequire(l.size() == 1);
  mRequire(!l.empty());
  it = l.begin();
  mRequire(*it == 1);
  ++it;
  mRequire(it == l.end());

  l.pop_front();

  mRequire(l.size() == 0);
  mRequire(l.empty());
  it = l.begin();
  mRequire(it == l.end());
}


mTestCase(list_copy) {
  List<int> l;
  l.push_front(1);
  l.push_front(2);
  l.push_front(3);

  List<int> l_copy;
  l_copy = l;

  mRequire(l.begin() != l_copy.begin());

  mRequire(!l.empty());
  auto it = l.begin();
  mRequire(*it == 3);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 1);
  ++it;
  mRequire(it == l.end());

  mRequire(!l_copy.empty());
  it = l_copy.begin();
  mRequire(*it == 3);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 1);
  ++it;
  mRequire(it == l_copy.end());
}


mTestCase(list_move) {
  List<int> l;
  l.push_front(1);
  l.push_front(2);
  l.push_front(3);

  List<int> l_move(move(l));

  mRequire(l.begin() != l_move.begin());
  mRequire(l.empty());
  mRequire(l.end() == l.begin());

  mRequire(!l_move.empty());
  auto it = l_move.begin();
  mRequire(*it == 3);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 1);
  ++it;
  mRequire(it == l_move.end());
}


mTestCase(list_conv) {
  List<int> l;
  parse_str("[1, 2, 3]", l);

  mRequire(l.size() == 3);
  mRequire(!l.empty());
  auto it = l.begin();
  mRequire(*it == 1);
  ++it;
  mRequire(*it == 2);
  ++it;
  mRequire(*it == 3);
  ++it;
  mRequire(it == l.end());

  mRequireEqStr(fmt(l), "[1, 2, 3]");

  l.clear();
  parse_str("[5]", l);
  mRequire(l.size() == 1);

  it = l.begin();
  mRequire(*it == 5);
  ++it;
  mRequire(it == l.end());

  mRequireEqStr(fmt(l), "[5]");

  l.clear();
  mRequireEqStr(fmt(l), "[]");

  parse_str("[]", l);
  mRequire(l.size() == 0);
  mRequire(l.empty());
}


mTestCase(list_backwards) {
  List<int> l;
  parse_str("[1, 2, 3]", l);

  auto it = l.end();
  mLogDebug("end()");
  mRequire(not it.has_value());

  mLogDebug("3");
  --it;
  mRequire(it.has_value());
  mRequire(*it == 3);

  mLogDebug("2");
  --it;
  mRequire(it.has_value());
  mRequire(*it == 2);

  mLogDebug("1");
  --it;
  mRequire(it.has_value());
  mRequire(*it == 1);

  mLogDebug("begin()");
  --it;
  mRequire(not it.has_value());

  mLogDebug("1");
  ++it;
  mRequire(it.has_value());
  mRequire(*it == 1);
}


mTestCase(list_remove_next) {
  List<int> l;
  parse_str("[1, 2, 3]", l);

  auto it = l.end();
  --it;  // 3
  --it;  // 2
  mRequire(it.has_value());
  mRequire(*it == 2);
  mRequire(l.remove(it) == 2);
  ++it;
  mRequire(*it == 3);
}


mTestCase(list_remove_prev) {
  List<int> l;
  parse_str("[1, 2, 3]", l);

  auto it = l.end();
  --it;  // 3
  --it;  // 2
  mRequire(it.has_value());
  mRequire(*it == 2);
  mRequire(l.remove(it) == 2);
  --it;
  mRequire(*it == 1);
}
