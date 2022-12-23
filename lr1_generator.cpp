#include <set>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

typedef pair<string, vector<string>> Production; // ����ʽ�����ݽṹ����Ϊstring���Ҳ�Ϊvector<string>

// �ʷ�������token������Ԥ������ж�
string all_terminals[] = {
    "_AUTO", "_BREAK", "_CASE", "_CHAR", "_CONST", "_CONTINUE", "_DEFAULT", "_DO", "_DOUBLE",
    "_ELSE", "_ENUM", "_EXTERN", "_FLOAT", "_FOR", "_GOTO", "_IF", "_INT", "_LONG", "_REGISTER",
    "_RETURN", "_SHORT", "_SIGNED", "_SIZEOF", "_STATIC", "_STRUCT", "_SWITCH", "_TYPEDEF",
    "_UNION", "_UNSIGNED", "_VOID", "_VOLATILE", "_WHILE", "+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
    ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
    ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'", "INTCON", "CHARCON", "STRCON", "IDENFR"}; // length = 7

struct Project
{                       // ��Ŀ��
  string left;          // ��
  vector<string> right; // �Ҳ�
  set<string> expect;   // չ����

  // ���������
  const bool operator<(const Project &p) const
  {
    if (left < p.left)
      return true;
    if (left > p.left)
      return false;
    if (right < p.right)
      return true;
    if (right > p.right)
      return false;
    if (expect < p.expect)
      return true;
    return false;
  }

  const bool operator==(const Project &p) const
  {
    if (left == p.left && right == p.right && expect == p.expect)
      return true;
    return false;
  }
};

struct ItemSet
{
  // ���ڲ�����Ŀ���Ĳ���ʽ
  vector<string> terminal;        // �ս������Ӧ�Ϸ���all_terminals
  set<string> non_terminal;       // ���ս����ʹ��setȥ�ش洢
  vector<string> all_symbols;     // ���еķ��ţ������ս���ͷ��ս��
  map<string, int> symbol_hash;   // ���Ź�ϣ����ǰ�ķ���Ϊ�ս������Ӧ�ʷ�������token������ķ���Ϊ���ս��
  vector<Production> productions; // ���еĲ���ʽ
  set<Production> items;          // ���е���Ŀ���������мӡ���Ĳ���ʽ

  void gen_poj()
  {
    ifstream fin("./Grammar.txt"); // ��ȡ�ķ�
    // ���ļ���ʧ�ܣ����������Ϣ���˳�
    if (!fin)
    {
      cout << "ERROR: grammar file not found" << endl;
      exit(0);
    }
    string a, b;                   // ����ʽ��(a)�ͼ�ͷ(b)��������b
    vector<string> c;              // ����ʽ�Ҳ���ʹ��vector�洢
    while (fin >> a >> b)
    {
      non_terminal.insert(a); // ����ʽ��һ���Ƿ��ս�������뵽���ս��������
      string str;
      getline(fin, str); // ��ȡ����ʽ�Ҳ�������ʣ�ಿ�֣�
      stringstream ss;   // ���ڷָ����ʽ�Ҳ�
      ss.str(str);       // ������ʽ�Ҳ���ֵ��ss
      c.clear();         // ���c��������һ�δ洢����ʽ�Ҳ�
      while (ss >> str)  // �ָ����ʽ�Ҳ�
      {
        c.push_back(str); // ��ÿ������ʽ�Ҳ��ķ��Ŵ���c
        bool flag = true; // �ж��Ƿ����ս��(���ս����Сд��ĸ���»������)
        for (int i = 0; i < str.size(); i++)
          if (!(str[i] >= 'a' && str[i] <= 'z' || str[i] == '_')) // �������Сд��ĸ���»��ߣ�˵�����ս��
          {
            flag = false;
            break;
          }
        if (flag)
          non_terminal.insert(str); // ������ս�������뵽���ս��������
      }
      productions.push_back(Production(a, c)); // ������ʽ���뵽����ʽ������
    }
    // �����е��ս�����뵽�ս��������
    for (int i = 0; i < 76; i++)
      terminal.push_back(all_terminals[i]);
    terminal.push_back("$");        // ���������$
    non_terminal.erase("program'"); // ɾ��program'
    for (auto it : terminal)        // �����е��ս�����뵽all_symbols��
      all_symbols.push_back(it);
    for (auto it : non_terminal) // �����еķ��ս�����뵽all_symbols��
      all_symbols.push_back(it);
    for (int i = 0; i < all_symbols.size(); i++) // �����еķ���ת��Ϊ��Ӧ�Ĺ�ϣֵ����0��ʼ
      symbol_hash[all_symbols[i]] = i;
    // ��ӡ��ͷ
    for (int i = 0; i < all_symbols.size(); i++)
      printf("\"%s\", ", all_symbols[i].c_str());
    printf("\n");
    for (auto it : productions) // �������еĲ���ʽ���������е���Ŀ��
    {
      a = it.first, c = it.second;        // ȡ������ʽ�󲿺��Ҳ�
      for (int i = 0; i <= c.size(); i++) // ����ʽ�Ҳ���ÿ��λ�ö����Լӡ�
      {
        vector<string> d = c;
        d.insert(d.begin() + i, ".");   // �ڵ�i��λ�üӡ�
        items.insert(Production(a, d)); // ���ӡ���Ĳ���ʽ���뵽��Ŀ����
      }
    }
  }
};

struct LR1
{
  // ����lr��
  ItemSet is;                                             // ��Ŀ����Ϣ
  vector<set<Project>> can_col = vector<set<Project>>(1); // ����ʽ��Ŀ���淶�壬��Ŀ���淶���Ӣ��Ϊcanonical_collection
  string lr1[560][115];                                   // lr1������

  set<string> _first(vector<string> X)
  {
    // ��ȡ���ս��X��FIRST��
    set<string> res;                                                             // ���ڱ��淵�ؽ��
    if (find(is.terminal.begin(), is.terminal.end(), X[0]) != is.terminal.end()) // ������ս����ֱ�Ӳ���FIRST��������
    {
      res.insert(X[0]);
      return res;
    }
    else
    {
      for (int j = 0; j < is.productions.size(); j++) // �������еĲ���ʽ
      {
        if (is.productions[j].first == X[0]) // �ҵ�����ʽ��ΪX�Ĳ���ʽ
        {
          if (find(is.terminal.begin(), is.terminal.end(), is.productions[j].second[0]) != is.terminal.end()) // �����һ�����ս��
            res.insert(is.productions[j].second[0]);                                                          // ������뵽res��
          else
          {
            set<string> t = _first(is.productions[j].second); // ����ݹ���FIRST��
            res.insert(t.begin(), t.end());                   // ��������뵽res��
          }
        }
      }
    }
    return res; // ���ؽ��
  }

  set<Project> _go(set<Project> I, string X)
  {
    // GO����
    set<Project> res; // ���ڱ��淵�ؽ��
    for (auto it : I) // ������Ŀ��I�е�ÿһ����Ŀ
    {
      vector<string> vs = it.right;               // ȡ����Ŀ���Ҳ�
      auto pos = find(vs.begin(), vs.end(), "."); // �ҵ�����λ��
      if (pos == vs.end() - 1)                    // ����������һ����������һ����Ŀ
        continue;
      if (*(pos + 1) == X) // ���������ķ�����X
      {
        swap(*pos, *(pos + 1));                      // �������ͺ����һ���ַ���
        res.insert(Project{it.left, vs, it.expect}); // ��������Ĳ���ʽ���뵽res��
      }
    }
    return res; // ���ؽ��
  }

  set<Project> _closure(set<Project> I)
  {
    // �հ�����
    while (true)
    {
      bool update = false; // �����ж��Ƿ����
      for (auto it : I)    // ������Ŀ��I�е�ÿһ����Ŀ
      {
        vector<string> X = it.right;              // ȡ����Ŀ���Ҳ�
        auto pos = find(X.begin(), X.end(), "."); // �ҵ�����λ��
        if (pos == X.end() - 1)                   // ����������һ������A->������������һ����Ŀ
          continue;
        string c = *(pos + 1); // ȡ���������һ���ַ�
        if (find(is.terminal.begin(), is.terminal.end(), c) != is.terminal.end())
          continue;                  // ���c���ս������A->����a��������һ����Ŀ
        X.erase(X.begin(), pos + 2); // ����A->����B�£�ɾ����������ַ��͡�
        string last;                 // ��¼��һ�α����FIRST���ĵ�һ���ַ���
        for (auto ite : it.expect)   // ������Ŀ��expect������FIRST(��a)
        {
          X.push_back(ite); // ��expect���е�ÿһ���ַ������뵽X��ĩβ
          if (last == X[0]) // �����һ�α����FIRST���ĵ�һ���ַ����������ͬ��������һ���ַ���
            continue;
          else
            last = X[0];                                // ���򣬸���last
          set<string> First = _first(X);                // ��ȡX��FIRST��
          X.pop_back();                                 // ɾ��X��ĩβ�ַ�������ΪX��һ����ʱ����������ı���Ŀ��right
          for (auto it1 : is.items)                     // �������еĲ���ʽ
            if (it1.first == c && it1.second[0] == ".") // �������ʽ������c���Ҳ��ԡ���ͷ����B->����
            {
              set<string> temp; // ���ڱ���expect��
              Project p{it1.first, it1.second, temp};
              auto pos = I.lower_bound(p);                                                  // ���ֲ��ҵ�һ�����ڵ���p��λ��
              if (pos != I.end() && (*pos).left == it1.first && (*pos).right == it1.second) // ����ҵ��ˣ������expect��
              {
                p = *pos;
                I.erase(p);                                  // ɾ��ԭ������Ŀ���Ա���и���
                p.expect.insert(First.begin(), First.end()); // ����expect��
                I.insert(p);                                 // ������º����Ŀ
              }
              else
              {
                update = true;                                   // ���û�ҵ�����˵�����µ���Ŀ����Ҫ���룬����������Ŀ��
                I.insert(Project{it1.first, it1.second, First}); // �����µ���Ŀ
              }
            }
        }
      }
      if (!update) // ���û�и��£����˳�ѭ��
        break;
    }
    return I;
  }

  void gen_lr1()
  {
    for (auto it : is.items) // ����������Ŀ
    {
      if (it.first == "program'" && it.second[0] == ".") // �ҵ���Ŀprogram'->��S
      {
        set<string> temp;                                      // ���ڱ���expect��
        temp.insert("$");                                      // expect���м���$
        can_col[0].insert(Project{it.first, it.second, temp}); // ����Ŀ���뵽��һ����Ŀ����
        break;
      }
    }
    can_col[0] = _closure(can_col[0]);       // ��ȡ��һ����Ŀ���ıհ�
    for (int i = 0; i < can_col.size(); i++) // �������е���Ŀ��
    {
      cout << "�����ɵ�" << i + 1 << "����Ŀ����" << endl; // �����ʾ��Ϣ
      for (auto it : can_col[i])                           // ������Ŀ���壬���й�Լ
      {
        int len = it.right.size();    // ��Ŀ���Ҳ�����
        if (it.right[len - 1] == ".") // �����һ���ַ����ǡ������ɽ��й�Լ
        {
          it.right.erase(it.right.end() - 1);             // ����ɾ��
          Production p(it.left, it.right);                // �������ʽ
          for (int j = 0; j < is.productions.size(); j++) // �������еĲ���ʽ
          {
            if (is.productions[j] == p) // �������ʽ��ͬ�����ҵ���Ŀ�����ʽ
            {
              string t;
              if (j == 0) // ����ǵ�һ������ʽ����program'->��program����Ϊ������Ŀ������Ϊacc
                t = "acc";
              else
                t = "r" + to_string(j); // ����Ϊ��Լ��Ŀ�������Լ�Ķ���Ϊrj

              for (auto its : it.expect)
                lr1[i][is.symbol_hash[its]] = t; // ���������뵽lr1����
            }
          }
        }
      }

      for (auto X : is.all_symbols) // �������еķ���
      {
        set<Project> J = _go(can_col[i], X); // ��ȡ��Ŀ������ƽ���Ŀ��
        if (!J.empty())                      // ��J�ǿ�
        {
          J = _closure(J);                                    // ��ȡJ�ıհ�
          int k;                                              // ��¼J��λ��
          auto pos = find(can_col.begin(), can_col.end(), J); // ���Ѿ����ɵ���Ŀ�����в���J
          if (pos != can_col.end())                           // ������Ŀ�������ҵ���J
            k = pos - can_col.begin();                        // ��¼J��λ��
          else
          {
            k = can_col.size();   // ����J���µ���Ŀ������¼J��λ��
            can_col.push_back(J); // ��J���뵽��Ŀ������
          }
          int j = is.symbol_hash[X];                                                // ��¼X��λ��
          if (find(is.terminal.begin(), is.terminal.end(), X) != is.terminal.end()) // ���X���ս��
            lr1[i][j] = "s" + to_string(k);                                         // ��Ϊ�ƽ���Ŀ������Ϊsj
          else
            lr1[i][j] = to_string(k); // ����Ϊ��Լ��Ŀ��������goto���У���Ӧ״̬j
        }
      }
    }
  }

  void save_lr1()
  {
    // ���lr1���ļ�LR(1).txt�У����������������ȡ
    ofstream fout("./LR(1).txt");
    int n = can_col.size();        // ��Ŀ���ĸ���
    int m = is.all_symbols.size(); // ���ŵĸ���
    fout << n << " " << m << endl; // �����Ŀ���ĸ����ͷ��ŵĸ���
    // ���lr1���������
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
      {
        if (lr1[i][j] == "")
          fout << "err "; // ���lr1[i][j]Ϊ�գ������err������
        else
          fout << lr1[i][j] << ' ';
      }
      fout << endl; // ����
    }
  }

  void main()
  {
    // ������
    is.gen_poj();
    gen_lr1();
    save_lr1();
  }
};

int main()
{
  LR1 lr;
  lr.main();
  return 0;
}
