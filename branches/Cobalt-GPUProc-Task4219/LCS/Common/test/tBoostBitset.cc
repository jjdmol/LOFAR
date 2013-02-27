#include <lofar_config.h>
#include <iostream>
#include <sstream>
#include <boost/dynamic_bitset.hpp>

using namespace std;

int main(int, char*[]) {
  boost::dynamic_bitset<> x(25); // all 0's by default
  x[0] = 1;
  x[1] = 1;
  x[22] = 1;
  for (boost::dynamic_bitset<>::size_type i = 0; i < x.size(); ++i)
    std::cout << x[i];
  std::cout << "\n";
  std::cout << x << "\n";
  std::cout << "size:" << x.size() << "\n";
  std::cout << "count:" << x.count() << "\n";
  std::cout << "num_blocks:" << x.num_blocks() << "\n";
  std::cout << "max_size:" << x.max_size() << "\n";
  std::cout << "bits_per_block:" << x.bits_per_block << "\n";
  std::cout << "sizeof:" << sizeof(x) << "\n";

  stringstream	buf(stringstream::in | stringstream::out);
  buf << x;
  string  s = buf.str();
  cout << "s:" << s << endl;
  cout << "len(s):" << s.size() << endl;

  boost::dynamic_bitset<>  y(x.size());
  buf >> y;
  std::cout << "\nmemcpy to y...\n";
  std::cout << y << "\n";
  std::cout << "size:" << y.size() << "\n";
  std::cout << "count:" << y.count() << "\n";
  std::cout << "num_blocks:" << y.num_blocks() << "\n";
  std::cout << "max_size:" << y.max_size() << "\n";
  std::cout << "bits_per_block:" << y.bits_per_block << "\n";
  std::cout << "sizeof:" << sizeof(y) << "\n";

  vector<boost::dynamic_bitset<>::block_type>	abc;
  abc.resize(x.num_blocks());
  vector<boost::dynamic_bitset<>::block_type>::iterator	sIter(abc.begin());
  boost::to_block_range(x, sIter);
  cout << "AFTER to_block_range" << endl;

  boost::dynamic_bitset<>  z(x.size());
  boost::from_block_range(abc.begin(), abc.end(), z);
  cout << "AFTER from_block_range" << endl;

  std::cout << "\nblock copy to z...\n";
  std::cout << z << "\n";
  std::cout << "size:" << z.size() << "\n";
  std::cout << "count:" << z.count() << "\n";
  std::cout << "num_blocks:" << z.num_blocks() << "\n";
  std::cout << "max_size:" << z.max_size() << "\n";
  std::cout << "bits_per_block:" << z.bits_per_block << "\n";
  std::cout << "sizeof:" << sizeof(z) << "\n";

  

  return EXIT_SUCCESS;
}

