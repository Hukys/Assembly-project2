#include <cstdlib>
#include <fstream>
#include <iostream>

#include <string>
#include <vector>
using namespace std;

struct Predictor {
  int history[3] = {0, 0, 0};              // 3bit
  int state[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 2^3
  char Predict = 'N';
  char Outcome = 'N';
};

struct instruction {
  int entrynum;
  int ordernumber;
  string inst;
  bool isbranch = false;
};

string getope(string s) {
  string buf = "";
  // bool breakflag= false;
  for (int i = 2; i < s.size(); i++) {
    if (s[i] == ' ') {
      break;
    }
    buf = buf + s[i];
  }
  return buf;
};

int jumptobranch(string label, vector<instruction> AllInst) {
  int pos = 0;
  for (int i = 0; i < AllInst.size(); i++) {
    if (AllInst[i].isbranch) {
      // cout<<AllInst[i].inst<<" vs "<<label<<endl;
      if (AllInst[i].inst.compare(label) == 0) {
        pos = AllInst[i].ordernumber + 1;
        //cout << "jump to " << pos << endl;

        break;
      }
    }
  }
  return pos;
};

int main() {
  string present[4]={"SN","WN","WT","ST"};
  string mistake;
  int entrycount = 0;
  int pc = 0; // 當前 counter
  // cout << "Hello World!\n" << endl;
  cout << "input entry(entry > 0): " << endl;
  cin >> entrycount;

  vector<instruction> AllInst;

  vector<Predictor> predictor;
  for (int i = 0; i < entrycount; i++) {
    Predictor buffer;
    predictor.push_back(buffer);
  }

  ifstream readf("as.txt", ios::in);
  if (!readf.is_open())
    cout << "Open failed" << endl;
  else {
    string str;
    instruction buffer;
    int counter = 0;
    int order = 0;
    while (getline(readf, str)) {
      // cout << counter<< ' '<<str[0] << endl;
      if (counter == entrycount) { //讀入順便配對
        counter = 0;
      }
      if (!isalpha(str[0])) { // 為inst
        buffer.entrynum = counter;
        buffer.isbranch = false;
        counter++;
      } else {
        buffer.entrynum = counter;
        buffer.isbranch = true;
      }
      buffer.inst = str;
      buffer.ordernumber = order;
      order++;
      AllInst.push_back(buffer);
    }
    readf.close();
  }

  // for (int i = 0; i < AllInst.size(); i++) {
  //   // cout << AllInst[i].inst << ' ' << AllInst[i].entrynum << ' '
  //   //      << AllInst[i].ordernumber;
  //   if (AllInst[i].isbranch) {
  //     cout << "  is label";
  //   }
  //   cout << endl;
  // }

  bool done = false;
  int R[32] = {0};
  while (!done) {
    if (pc >= AllInst.size()) {
      cout << "go down to End  " << pc << endl;
      break;
    }
    mistake = "No";
    // cout<<pc<<endl;
    int nowEntry = AllInst[pc].entrynum;

    int posi = predictor[nowEntry].history[0] * 4 +
               predictor[nowEntry].history[1] * 2 +
               predictor[nowEntry].history[2] * 1;

    if (predictor[nowEntry].state[posi] < 2) {
      predictor[nowEntry].Predict = 'N';
    } else {
      predictor[nowEntry].Predict = 'T';
    } // 算出預測

    string instbuf = AllInst[pc].inst;
    string ope;

    int temp[3] = {0};
    string templabel = "";
    int historybuffer = 0;
    // cout << instbuf << " nowEntry=" << nowEntry << endl;
    if (!AllInst[pc].isbranch) { // inst
      ope = getope(instbuf);
      // cout << ope << endl;
      // cout<<ope<<endl;
      if (ope == "addi") {

        int j = 0;
        for (int i = 7; i < instbuf.length(); i++) {
          if (j < 2) {
            if (instbuf[i] == ',') {
              temp[j] = (instbuf[i - 1]) - '0';
              j++;
            }
          } else {
            templabel = templabel + instbuf[i];
          }
        }
        temp[j] = stoi(templabel);

        //cout << temp[0] << " " << temp[1] << " " << temp[2] << endl;

        R[temp[0]] = R[temp[1]] + temp[2];
        pc++;
        predictor[nowEntry].Outcome = 'N';
        historybuffer = 0;

      } else if (ope == "beq") {
        int j = 0;
        for (int i = 6; i < instbuf.length(); i++) {

          if (j < 2) {
            if (instbuf[i] == ',') {
              //cout << instbuf[i - 1] << endl;
              temp[j] = (instbuf[i - 1]) - '0';
              //  cout<<temp[j]<<endl;
              j++;
            }
          } else {
            templabel = templabel + instbuf[i];
          }
        }
        //cout << temp[0] << " " << temp[1] << " " << templabel << endl;
        if (R[temp[0]] == R[temp[1]]) { // taken
          predictor[nowEntry].Outcome = 'T';
          historybuffer = 1;
          pc = jumptobranch(templabel, AllInst);
        } else { // not taken
          predictor[nowEntry].Outcome = 'N';
          historybuffer = 0;
          pc++;
        }        
      } else {
        cout << ope << endl;
        done = true;
      }
      if (predictor[nowEntry].Outcome != predictor[nowEntry].Predict)
        mistake = "Yes";
      cout<<endl<<"nowEntry= "<<nowEntry<<"    "<<instbuf<<endl<<"Prediction: "<<predictor[nowEntry].Predict<<", Output: "<<predictor[nowEntry].Outcome<<" Misspredict: "<<mistake<<endl;
      for(int i=0;i<entrycount;i++){
        cout<<"Entry: "<< i<<" ("<<predictor[i].history[0]<<predictor[i].history[1]<<predictor[i].history[2]<<", "<<present[predictor[i].state[0]]<<", "<<present[predictor[i].state[1]]<<", "<<present[predictor[i].state[2]]<<", "<<present[predictor[i].state[3]]<<", "<<present[predictor[i].state[4]]<<", "<<present[predictor[i].state[5]]<<", "<<present[predictor[i].state[6]]<<", "<<present[predictor[i].state[7]]<<") "<<endl;
          }

      if (predictor[nowEntry].Outcome=='T'&& predictor[nowEntry].state[posi]<3)
        predictor[nowEntry].state[posi]++;
      else if (predictor[nowEntry].Outcome=='N'&& predictor[nowEntry].state[posi]>0)
        predictor[nowEntry].state[posi]--;
      
      predictor[nowEntry].history[0]=predictor[nowEntry].history[1];
      predictor[nowEntry].history[1]=predictor[nowEntry].history[2];
      predictor[nowEntry].history[2]=historybuffer;
      
      } else {
      // cout << pc << " is label so jump" << endl;
      pc++;
    }
    
  }
}
