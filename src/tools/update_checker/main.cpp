
#include "Define.h"
#include "Errors.h"
#include "Util.h"

#include <cstdio>
#include <string>
#include <fstream>

#include <boost/asio.hpp>

#include <openssl/sha.h>


static char const* crlf_str =
  "blub\r\n1234567890\r\n1234567890\r\n";

static char const* lf_str =
  "blub\n1234567890\n1234567890\n";

std::string ReadFile(std::string const& file)
{
    std::ifstream in(file.c_str());
    ASSERT(in.is_open() && "Could not read an update file.");

    auto const start_pos = in.tellg();
    in.ignore(std::numeric_limits<std::streamsize>::max());
    auto const char_count = in.gcount();
    in.seekg(start_pos);

    std::string update(char_count, char{});

    in.read(&(update[0]), update.size());
    in.close();
    return update;
}

std::string CalculateHash(std::string const& query)
{
    // Calculate a Sha1 hash based on query content.
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)query.c_str(), query.length(), (unsigned char*)&digest);

    return ByteArrayToHexStr(digest, SHA_DIGEST_LENGTH);
}

int main(int, char**)
{
  std::printf("");

  // Write the files
  {
    std::ofstream o("crlf_test.txt", std::fstream::binary);
    ASSERT(o.is_open());
    o << crlf_str;
    o.close();
  }
  {
    std::ofstream o("lf_test.txt", std::fstream::binary);
    ASSERT(o.is_open());
    o << lf_str;
    o.close();
  }

  auto crlf = ReadFile("crlf_test.txt");
  auto lf = ReadFile("lf_test.txt");

  if (lf == crlf)
    printf("[OK] textmode ok\n");
  else
  {
    printf("[ERROR] textmode failed (lf != crlf)\n");
    crlf = lf;
  }

  auto crlf_hash = CalculateHash(crlf);
  auto lf_hash = CalculateHash(lf);

  if (lf_hash == crlf_hash)
    printf("[OK] hashes are equal\n");
  else
  {
    printf("[ERROR] hashes mismatch (%s != %s)\n", lf_hash.c_str(), crlf_hash.c_str());
    crlf_hash = lf_hash;
  }

  if (lf_hash == "F6A91C958ECC2C68F4484F44A5C267B061FACAE5")
    printf("[OK] the hash is correct\n");
  else
    printf("[ERROR] The hash is wrong! (%s)\n", lf_hash.c_str());

  return 0;
}
