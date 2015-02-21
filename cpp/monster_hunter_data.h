#ifndef _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_
#define _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_

#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <map>
#include <unordered_map>

#include "helpers.h"
#include "parser.h"


namespace monster_avengers {

  struct Skill {
    int points;
    std::wstring name;
    std::wstring description;

    // Constructor from reading Tokenizer.
    Skill(parser::Tokenizer *tokenizer, 
          bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"DESCRIPTION" == token.value) {
            description = tokenizer->ExpectString();
          } else if (L"NAME" == token.value) {
            name = tokenizer->ExpectString();
          } else if (L"POINTS" == token.value) {
            points = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    } 

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Skill {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"description: %ls\n", description.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };

  struct SkillSystem {
    std::wstring name;
    std::vector<Skill> skills;

    // Constructor from reading Tokenizer.
    explicit SkillSystem(parser::Tokenizer *tokenizer,
                         bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;

      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"SYSTEM-NAME" == token.value) {
            name = tokenizer->ExpectString();
          } else if (L"SKILLS" == token.value) {
            skills = std::move(parser::ParseList<Skill>::Do(tokenizer));
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"SkillSystem {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skills: [\n");
      for (const Skill &skill : skills) {
        skill.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  namespace {
    std::map<std::wstring, int> skill_name_to_id;
    
    void UpdateSkillSystemLookUp(const std::vector<SkillSystem> &systems) {
      skill_name_to_id.clear();
      for (int i = 0; i < systems.size(); ++i) {
        skill_name_to_id[systems[i].name] = i;
      }
    }

    int LookUpSkillSystem(const std::wstring &name) {
      if (skill_name_to_id.empty()) {
        Log(ERROR, L"Skill systems are not loaded.");
      }
      auto it = skill_name_to_id.find(name);
      if (skill_name_to_id.end() != it) {
        return it->second;
      }
      return -1;
    }
  }
  
  struct Effect {
    int skill_id;
    int points;
    
    // Constructor from reading Tokenizer.
    Effect(parser::Tokenizer *tokenizer, 
           bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      skill_id = -1;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"SKILL-NAME" == token.value) {
            std::wstring skill_name = tokenizer->ExpectString();
            skill_id = LookUpSkillSystem(skill_name);
            if (-1 == skill_id) {
              Log(WARNING, L"Invalid skill system name: %ls", 
                  skill_name.c_str());
            }
          } else if (L"SKILL-POINT" == token.value) {
            points = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    } 

    Effect(int input_skill_id, int input_points) 
      : skill_id(input_skill_id), points(input_points) {}

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Effect {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skill_id: %d\n", skill_id);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  struct Jewel {
    std::wstring name;
    int holes;
    std::vector<Effect> effects;
    
    // Constructor from reading Tokenizer.
    explicit Jewel(parser::Tokenizer *tokenizer,
                   bool expect_open_paren = true) 
      : effects() {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;

      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = tokenizer->ExpectString();
          } else if (L"EFFECTS" == token.value) {
            effects = std::move(parser::ParseList<Effect>::Do(tokenizer));
          } else if (L"HOLES" == token.value) {
            holes = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }

      auto it = std::remove_if(effects.begin(), effects.end(),
                               [](const Effect &effect) {
                                 return -1 == effect.skill_id;
                               });
      for (int i = 0; i < effects.end() - it; ++i) {
        effects.pop_back();
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Jewel {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };

  enum WeaponType {
    MELEE = 0,
    RANGE,
    BOTH,
  };

  enum ArmorPart {
    HEAD = 0,
    BODY = 1,
    HANDS = 2,
    WAIST = 3,
    FEET = 4,
    GEAR = 5,
    AMULET = 6,
    PART_NUM
  };

  struct Armor {
    std::wstring name;
    ArmorPart part;
    WeaponType type;
    int rare;
    int defense;
    int holes;
    std::vector<Effect> effects;
    std::vector<std::wstring> material;
    // Whether the armor is created only for being multiplied.
    bool multiplied;
    // What is the base armor, in torso up case.
    int base;
    // The stuffed jewels
    std::unordered_map<int, int> jewels;
    
    Armor() = default;

    static Armor Amulet(int holes, std::vector<Effect> effects) {
      Armor armor;
      armor.name = L"Amulet";
      armor.type = BOTH;
      armor.rare = 10;
      armor.defense = 0;
      armor.holes = holes;
      armor.effects = std::move(effects);
      armor.part = AMULET;
      armor.multiplied = false;
      armor.base = -1;
      armor.jewels.clear();
      return armor;
    }
    
    // Constructor from reading Tokenizer.
    explicit Armor(parser::Tokenizer *tokenizer,
                   bool expect_open_paren = true) 
      : multiplied(false), base(-1), jewels() {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      std::vector<std::wstring> skill_names;
      std::vector<int> skill_points;
      parser::Token token;
      bool complete = false;
      bool name_detected = false;
      
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = tokenizer->ExpectString();
            name_detected = true;
          } else if (L"HOLES" == token.value) {
            holes = tokenizer->ExpectNumber();
          } else if (L"RANK" == token.value) {
            rare = tokenizer->ExpectNumber();
          } else if (L"TYPE" == token.value) {
            std::wstring type_string = tokenizer->ExpectString();
            if (L"melee" == type_string) {
              type = MELEE;
            } else if (L"range" == type_string) {
              type = RANGE;
            } else {
              type = BOTH;
            }
          } else if (L"DEFENSE" == token.value) {
            defense = tokenizer->ExpectNumber();
          } else if (L"EFFECTIVE-POINTS" == token.value) {
            skill_points = std::move(parser::ParseList<int>::Do(tokenizer));
          } else if (L"EFFECTIVE-SKILLS" == token.value) {
            skill_names = 
              std::move(parser::ParseList<std::wstring>::Do(tokenizer));
          } else if (L"MATERIAL" == token.value) { 
            material = std::move(parser::ParseList<std::wstring>::Do(tokenizer));
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }

      CHECK(skill_names.size() == skill_points.size());

      for (int i = 0; i < skill_names.size(); ++i) {
        effects.emplace_back(LookUpSkillSystem(skill_names[i]), 
                             skill_points[i]);
      }
      
      if (!name_detected) name = L"";
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Armor {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"type: %ls\n", BOTH == type ? L"BOTH" 
              : (MELEE == type ? L"MELEE" : L"RANGE"));
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"defense: %d\n", defense);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"rare: %d\n", rare);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  class DataSet {
  public:
    int torso_up_id;
    
    DataSet(const std::string &data_folder) 
      : skill_systems_(), jewels_(), armors_(),
        armor_indices_by_parts_() {
      // Skills:
      torso_up_id = -1;
      {
        const std::string path = data_folder + "/skills.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        skill_systems_ = 
          std::move(parser::ParseList<SkillSystem>::Do(&tokenizer));

        // Search for torso_up ID. The torso_up skill is the one with
        // points = 0.
        for (int id = 0; id < skill_systems_.size(); ++id) {
          for (const Skill &skill : skill_systems_[id].skills) {
            if (0 == skill.points) {
              torso_up_id = id;
              break;
            }
          }
          if (torso_up_id > 0) {
            break;
          }
        }
        
        if (torso_up_id < 0) {
          Log(ERROR, L"Torso Up not found!\n");
          exit(-1);
        }
        UpdateSkillSystemLookUp(skill_systems_);
      }

      // Jewels:
      {
        const std::string path = data_folder + "/jewels.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        jewels_ = 
          std::move(parser::ParseList<Jewel>::Do(&tokenizer));
      }

      // Armors: (including amulet)
      armor_indices_by_parts_.resize(PART_NUM);
      ReadArmors<HEAD>(data_folder + "/helms.lisp");
      ReadArmors<BODY>(data_folder + "/cuirasses.lisp");
      ReadArmors<HANDS>(data_folder + "/gloves.lisp");
      ReadArmors<WAIST>(data_folder + "/cuisses.lisp");
      ReadArmors<FEET>(data_folder + "/sabatons.lisp");
      ReadArmors<GEAR>(data_folder + "/gears.lisp");
      LoadAmulet();
      reserved_armor_count_ = static_cast<int>(armors_.size());
    }

    inline const std::vector<Jewel> &jewels() const {
      return jewels_;
    }

    inline const Jewel &jewel(int id) const {
      return jewels_[id];
    }

    inline const std::vector<int> &ArmorIds(ArmorPart part) const {
      return armor_indices_by_parts_[part];
    }

    inline const Armor &armor(int id) const {
      return armors_[id];
    }

    inline const Armor &armor(ArmorPart part, int id) const {
      return armors_[armor_indices_by_parts_[part][id]];
    }

    inline bool ProvidesTorsoUp(ArmorPart part, int id) const {
      const Armor &armor = armors_[armor_indices_by_parts_[part][id]];
      return  1 == armor.effects.size() &&
        armor.effects[0].skill_id == torso_up_id;
    }

    inline bool ProvidesTorsoUp(int id) const {
      const Armor &armor = armors_[id];
      return  1 == armor.effects.size() &&
        armor.effects[0].skill_id == torso_up_id;
    }

    inline const std::vector<Armor> &armors() const {
      return armors_;
    }

    inline const SkillSystem &skill_system(int id) const {
      return skill_systems_[id];
    }

    inline void AddExtraArmor(ArmorPart part, const Armor &armor) {
      armor_indices_by_parts_[part].push_back(armors_.size());
      armors_.push_back(armor);
    }

    inline void ClearExtraArmor() {
      while (armors_.size() > reserved_armor_count_) {
        armors_.pop_back();
      }
    }

    void PrintSkillSystems() {
      for (int i = 0; i < skill_systems_.size(); ++i) {
        wprintf(L"%d: %ls\n", i, skill_systems_[i].name.c_str());
      }
    }

    double EffectScore(const Effect &effect) {
      int armor_count = 0;
      for (int i = 0; i < reserved_armor_count_; ++i) {
        for (const Effect &armor_effect : armors_[i].effects) {
          if (armor_effect.skill_id == effect.skill_id) {
            armor_count++;
            break;
          }
        }
      }

      double jewel_index = 0;
      for (const Jewel &jewel : jewels_) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (jewel_effect.skill_id == effect.skill_id) {
            jewel_index += 
              static_cast<double>(jewel_effect.points) / jewel.holes *
              std::exp(-jewel.holes * 0.1);

          }
        }
      }
      
      return std::exp(0.1 * jewel_index - 0.3 * effect.points)
        * armor_count;
    }

    void Summarize() {
      Log(INFO, L"Skill Systems: %lld", skill_systems_.size());
      Log(INFO, L"Jewels: %lld", jewels_.size());
      Log(INFO, L"Armors: %lld", armors_.size());
      Log(INFO, L" - HELMS: %lld", armor_indices_by_parts_[HEAD].size());
      Log(INFO, L" - CUIRASSES: %lld", armor_indices_by_parts_[BODY].size());
      Log(INFO, L" - GLOVES: %lld", armor_indices_by_parts_[HANDS].size());
      Log(INFO, L" - CUISSES: %lld", armor_indices_by_parts_[WAIST].size());
      Log(INFO, L" - SABATONS: %lld", armor_indices_by_parts_[FEET].size());
      Log(INFO, L" - GEARS: %lld", armor_indices_by_parts_[GEAR].size());
      Log(INFO, L" - AMULETS: %lld", armor_indices_by_parts_[AMULET].size());
    }
    
  private:
    
    template <ArmorPart Part>
    void ReadArmors(const std::string &path) {
      auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
      std::vector<Armor> armor_list = 
        std::move(parser::ParseList<Armor>::Do(&tokenizer));
      auto it = std::remove_if(armor_list.begin(),
                               armor_list.end(),
                               [](const Armor &armor) {
                                 return armor.name.empty();
                               });
      for (int i = 0; i < armor_list.end() - it; ++i) {
        armor_list.pop_back();
      }
      for (const Armor &armor : armor_list) {
        armors_.push_back(armor);
        armors_.back().part = Part;
        if (HEAD == Part || GEAR == Part) {
          armors_.back().type = BOTH;
        }
        armor_indices_by_parts_[Part].push_back(armors_.size() - 1);
      }
    }

    void LoadAmulet() {
      armors_.push_back(Armor::Amulet(0, {}));
      armor_indices_by_parts_[AMULET].push_back(armors_.size() - 1);
    }
    
    std::vector<SkillSystem> skill_systems_;
    std::vector<Jewel> jewels_;
    std::vector<Armor> armors_;
    int reserved_armor_count_;
    std::vector<std::vector<int> > armor_indices_by_parts_;
  };

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_
