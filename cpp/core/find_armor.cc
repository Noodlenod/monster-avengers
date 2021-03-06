#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "supp/helpers.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  CHECK(2 <= argc);
  ArmorUp armor_up(argv[1]);
  if (argc < 3) {
    armor_up.ListSkills();
  } else {
    Query query;
    CHECK_SUCCESS(Query::ParseFile(argv[2], &query));
    armor_up.SearchAndOutput(query, 10);
  }
  return 0;
}
