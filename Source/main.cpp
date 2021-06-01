#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nfd.h>
#include <nlohmann/json.hpp>

#include <array>
#include <cmath>
#include <fstream>


using json = nlohmann::json;

constexpr size_t normal_text_length = 32;
constexpr size_t multiline_text_length = 256;
constexpr size_t extended_multiline_text_length = 512;
constexpr size_t name_text_length = 128;

const std::string version_string = "Character Sheet v0.1 5e Damien Henderson";
struct SkillEntry
{
	bool proficient{ false };
	int* base_attribute{ nullptr };
	char base_attribute_string[normal_text_length]{ "" };
	char skill_name[normal_text_length]{ "" };
};


struct SpellSlot
{
	bool used{ false };
	int level{ 1 };
	char slot_name[normal_text_length]{"Spell Slot"};

	

};
struct SavingThrow
{
	bool proficient{ false };
	int* base_attribute{ nullptr };
	char base_attribute_name[normal_text_length]{ "" };
};
void CopyJsonStringToString(const json& j, std::string key, char* dest, size_t dest_size)
{
	auto tmp = j[key].get<std::string>();
	size_t to_copy = std::min(dest_size, tmp.size());
	tmp.copy(dest, to_copy);
	if (dest_size < tmp.size())
	{
		dest[dest_size - 1] = '\0';
	}


}

void to_json(json& j, const SpellSlot& slot) 
{
	
	j["Slot Name"] = slot.slot_name;
	j["Used"] = slot.used;
	j["Level"] = slot.level;
}

void from_json(const json& j, SpellSlot& slot) 
{
	CopyJsonStringToString(j, "Slot Name", slot.slot_name, normal_text_length);
	slot.used = j["Used"].get<bool>();
	slot.level = j["Level"].get<int>();
}

enum SkillID
{
	Acrobatics = 0,
	AnimalHandling,
	Arcana,
	Athletics,
	Deception,
	History,
	Insight,
	Intimidation,
	Investigation,
	Medicine,
	Nature,
	Perception,
	Performance,
	Persuasion,
	Religion,
	SleightOfHand,
	Stealth,
	Survival
};

class CharacterSheet
{
public:
	

	CharacterSheet()
	{
		InitString(character_name, name_text_length);
		InitString(class_name, normal_text_length);
		InitString(background, normal_text_length);
		InitString(party_name, name_text_length);
		InitString(race, normal_text_length);
		InitString(experience, normal_text_length);
		InitString(languages, multiline_text_length);
		InitString(feats, multiline_text_length);
		InitString(inventory, multiline_text_length);
		InitString(spellbook, extended_multiline_text_length);

		alignment_strings = { "Lawful Good", "Neutral Good", "Chaotic Good", "Lawful Neutral", "Neutral", "Chaotic Neutral", "Lawful Evil", "Neutral Evil", "Chaotic Evil" };
		ability_short_strings = { "STR", "DEX", "CON", "INT", "WIS", "CHA" };
		die_strings = { "d4", "d6", "d8", "d10", "d12", "d20" };
		sight_strings = { "Normal", "Darkvision", "Blindsight", "Truesight" };
		character_level = 1;

		proficiency = 1 + (int)std::ceil(character_level / 4.0);

		for (auto& it : skills)
		{
			it.base_attribute = &strength;
		}
		FillSkillsTable();
		FillSavingThrowTable();
		auto& perception_skill = skills[Perception];
		int perception_mod = ((*perception_skill.base_attribute - 10) / 2) + (proficiency * (int)perception_skill.proficient);
		passive_perception = 10 + perception_mod;
	}
	~CharacterSheet()
	{
		for (auto& it : strings)
		{
			delete[] it;
		}
	}


	void DoGUI(size_t width, size_t height)
	{
		
		const int min_level = 1;
		// Equations for proficiency still work for the level 50 variant rule
		const int max_level = 20;

		ImGui::Begin("Character Sheet", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(width, height));
		
		if (ImGui::Button("Save Character Sheet"))
		{
			// Save Sheet Here
			nfdchar_t* outPath = nullptr;
			nfdresult_t result = NFD_SaveDialog("json", nullptr, &outPath);
			if (result == NFD_OKAY)
			{
				std::ofstream json_file(outPath);
				json j;
				SaveToJson(j);
				json_file << j;
				json_file.close();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Character Sheet"))
		{
			// Load Sheet Here
			nfdchar_t* outPath = nullptr;
			nfdresult_t result = NFD_OpenDialog("json", nullptr, &outPath);

			if (result == NFD_OKAY)
			{
				std::ifstream json_file(outPath);
				json j;
				json_file >> j;
				if (j.count("Version"))
				{
					if (j["Version"].get<std::string>() == version_string)
					{
						LoadFromJson(j);
					}
				}
				
				json_file.close();
			}
		}


		ImGui::InputText("Character Name", character_name, name_text_length);

		ImGui::SliderInt("Level", &character_level, min_level, max_level);
		
		ImGui::InputText("Class", class_name, normal_text_length);

		ImGui::InputText("Background", background, normal_text_length);

		ImGui::InputText("Party Name", party_name , name_text_length);

		ImGui::InputText("Race", race, normal_text_length);

		ImGui::Combo("Alignment", &alignment, alignment_strings.data(), alignment_strings.size());

		ImGui::InputText("Experience", experience, normal_text_length);

		proficiency = 1 + (int)std::ceil(character_level / 4.0);

		ImGui::Text("Proficiency Bonus [%d]", proficiency);

		ImGui::InputInt("Inspiration", &inspiration, 1, 1);

		ImGui::InputInt("Initiative", &initiative, 1, 1);

		ImGui::InputInt("Armour Class", &armour_class, 1, 1);

		ImGui::InputInt("Speed", &speed, 1, 1);

		ImGui::InputInt("Hit Points", &current_hit_points, 1, 1);

		ImGui::InputInt("Max Hit Points", &max_hit_points, 1, 1);

		ImGui::Combo("Hit Die", &hit_die, die_strings.data(), die_strings.size());

		ImGui::Text("Max Hit Dice: %d %s", character_level, die_strings[hit_die]);

		ImGui::InputInt("Hit Dice", &hit_dice, 1, 1);

		hit_dice = std::min(std::max(0, hit_dice), character_level);

		ImGui::Combo("Spellcasting Ability", &spellcasting_ability, ability_short_strings.data(), ability_short_strings.size());

		ImGui::InputInt("Spell Save DC", &spell_save_dc, 1, 1);

		ImGui::InputInt("Spell Attack Bonus", &spell_attack_bonus, 1, 1);
		
		inspiration = std::max(inspiration, 0);

		if (ImGui::CollapsingHeader("Ability Scores"))
		{
			DoAttributeGUI("Strength", &strength);

			DoAttributeGUI("Dexterity", &dexterity);

			DoAttributeGUI("Constitution", &constitution);

			DoAttributeGUI("Intelligence", &intelligence);

			DoAttributeGUI("Wisdom", &wisdom);

			DoAttributeGUI("Charisma", &charisma);
		}
		if (ImGui::CollapsingHeader("Saving Throws"))
		{
			for (auto& saving_throw : saving_throws)
			{
				DoSavingThrowGUI(saving_throw);
			}
		}
		if (ImGui::CollapsingHeader("Skills"))
		{
			for (auto& it : skills)
			{
				DoSkillGUI(it);
			}

		}
		
		ImGui::InputInt("Passive Perception", &passive_perception, 1, 1);
		passive_perception = std::max(passive_perception, 0);
		if (ImGui::Button("Recalculate Passive Perception"))
		{
			auto& perception_skill = skills[Perception];
			int perception_mod = ((*perception_skill.base_attribute - 10) / 2) + (proficiency * (int)perception_skill.proficient);
			passive_perception = 10 + perception_mod;
		}

		if (ImGui::CollapsingHeader("Spells"))
		{
			ImGui::InputText("Slot Name", edit_slot_name, normal_text_length);
			ImGui::SliderInt("Slot Level", &edit_slot_level, 1, 10);

			if (ImGui::Button("Add Slot") && edit_slot_name[0] != '\0')
			{
				// TODO: Make spell slot names unique
				SpellSlot slot;
				memcpy(slot.slot_name, edit_slot_name, normal_text_length);
				slot.level = edit_slot_level;
				spell_slots.push_back(slot);
			}
			for (auto& it = spell_slots.begin(); it != spell_slots.end();)
			{
				bool should_erase = DoSpellSlotGUI(*it);
				if (should_erase)
				{
					it = spell_slots.erase(it);
				}
				else
				{
					++it;
				}
			}

			ImGui::InputTextMultiline("Spellbook", spellbook, extended_multiline_text_length);
		}
		if (ImGui::CollapsingHeader("Character Info"))
		{
			ImGui::InputTextMultiline("Languages", languages, multiline_text_length);

			ImGui::InputTextMultiline("Feats", feats, multiline_text_length);

			ImGui::InputTextMultiline("Inventory", inventory, multiline_text_length);

			ImGui::Combo("Sight", &sight, sight_strings.data(), sight_strings.size());
		}
		ImGui::End();

	}
private:
	// Helper Function
	void InitString(char*& string, size_t length)
	{
		if (string != nullptr)
		{
			// Don't allocate strings that aren't null, still track it though
			strings.push_back(string);
			return;
		}

		string = new char[length];
		memset(string, '\0', length);

		strings.push_back(string);
	}

	void DoAttributeGUI(const char* attribute_name, int* value)
	{
		const int min_attribute_score = 1;
		const int max_attribute_score = 30;
		
		ImGui::SliderInt(attribute_name, value, min_attribute_score, max_attribute_score);
		ImGui::SameLine();
		int modifier = (*value - 10) / 2;
		ImGui::Text(modifier > 0 ? "+%d" : "%d", modifier);
	}

	void DoSkillGUI(SkillEntry& skill)
	{
		char buf[name_text_length];
		sprintf(buf, "###%s_Proficiency", skill.skill_name);
		ImGui::Checkbox(buf, &skill.proficient);
		ImGui::SameLine();
		int skill_mod = ((*skill.base_attribute - 10) / 2) + (proficiency * (int)skill.proficient);
		ImGui::Text("%s: %d (%s)", skill.skill_name, skill_mod, skill.base_attribute_string);
	}
	
	bool DoSpellSlotGUI(SpellSlot& spell_slot)
	{
		char buf[name_text_length];

		ImGui::Checkbox(spell_slot.slot_name, &spell_slot.used);

		ImGui::SameLine();

		ImGui::Text("Level %d", spell_slot.level);

		ImGui::SameLine();

		
		sprintf(buf, "Remove###%s%d", spell_slot.slot_name, spell_slot.level);
		
		if (ImGui::Button(buf))
		{
			return true;
		}
		return false;
	}
	
	void DoSavingThrowGUI(SavingThrow& saving_throw)
	{
		char buf[name_text_length];
		sprintf(buf, "###%s_Save", saving_throw.base_attribute_name);
		ImGui::Checkbox(buf, &saving_throw.proficient);
		ImGui::SameLine();
		int save_mod = ((*saving_throw.base_attribute - 10) / 2) + (proficiency * (int)saving_throw.proficient);
		ImGui::Text("%s: %d (%s)", saving_throw.base_attribute_name, save_mod, saving_throw.base_attribute_name);
	}
	void FillSkillsTable()
	{
		int idx = 0;
		skills[idx++] = { false, &dexterity, "DEX", "Acrobatics" };
		skills[idx++] = { false, &wisdom, "WIS", "Animal Handling" };
		skills[idx++] = { false, &intelligence, "INT", "Arcana" };
		skills[idx++] = { false, &strength, "STR", "Athletics" };
		skills[idx++] = { false, &charisma, "CHA", "Deception" };
		skills[idx++] = { false, &intelligence, "INT", "History" };
		skills[idx++] = { false, &wisdom, "WIS", "Insight" };
		skills[idx++] = { false, &charisma, "CHA", "Intimidation" };
		skills[idx++] = { false, &intelligence, "INT", "Investigation" };
		skills[idx++] = { false, &wisdom, "WIS", "Medicine" };
		skills[idx++] = { false, &intelligence, "INT", "Nature" };
		skills[idx++] = { false, &wisdom, "WIS", "Perception" };
		skills[idx++] = { false, &charisma, "CHA", "Performance" };
		skills[idx++] = { false, &charisma, "CHA", "Persuasion" };
		skills[idx++] = { false, &intelligence, "INT", "Religion" };
		skills[idx++] = { false, &dexterity, "DEX", "Sleight of Hand" };
		skills[idx++] = { false, &dexterity, "DEX", "Stealth" };
		skills[idx++] = { false, &wisdom, "WIS", "Survival" };
	}
	void FillSavingThrowTable()
	{
		saving_throws[0] = { false, &strength, "Strength" };
		saving_throws[1] = { false, &dexterity, "Dexterity" };
		saving_throws[2] = { false, &constitution, "Constitution" };
		saving_throws[3] = { false, &intelligence, "Intelligence" };
		saving_throws[4] = { false, &wisdom, "Wisdom" };
		saving_throws[5] = { false, &charisma, "Charisma" };
	}
	void LoadFromJson(json& json_archive)
	{
		

		CopyJsonStringToString(json_archive, "Name", character_name, name_text_length);
		CopyJsonStringToString(json_archive, "Class", class_name, normal_text_length);

		character_level = json_archive["Level"].get<int>();

		CopyJsonStringToString(json_archive, "Background", background, normal_text_length);
		CopyJsonStringToString(json_archive, "Party", party_name, name_text_length);
		CopyJsonStringToString(json_archive, "Race", race, normal_text_length);
		CopyJsonStringToString(json_archive, "Experience", experience, normal_text_length);

		alignment = json_archive["Alignment"].get<int>();
		inspiration = json_archive["Inspiration"].get<int>();
		initiative = json_archive["Initiative"].get<int>();

		strength = json_archive["Attributes"]["Strength"].get<int>();
		dexterity = json_archive["Attributes"]["Dexterity"].get<int>();
		constitution = json_archive["Attributes"]["Constitution"].get<int>();
		intelligence = json_archive["Attributes"]["Intelligence"].get<int>();
		wisdom = json_archive["Attributes"]["Wisdom"].get<int>();
		charisma = json_archive["Attributes"]["Charisma"].get<int>();

		passive_perception = json_archive["Passive Perception"].get<int>();

		armour_class = json_archive["Armour Class"].get<int>();

		speed = json_archive["Speed"].get<int>();

		max_hit_points = json_archive["Max Hit Points"].get<int>();

		current_hit_points = json_archive["Hit Points"].get<int>();

		hit_die = json_archive["Hit Die"].get<int>();

		hit_dice = json_archive["Current Hit Dice"].get<int>();

		spellcasting_ability = json_archive["Spellcasting Ability"].get<int>();

		spell_save_dc = json_archive["Spell Save DC"].get<int>();

		spell_attack_bonus = json_archive["Spell Attack Bonus"].get<int>();
		
		for (size_t i = 0; i < skills.size(); i++)
		{
			skills[i].proficient = json_archive["Proficiencies"][i].get<bool>();
		}
		for (size_t i = 0; i < saving_throws.size(); i++)
		{
			saving_throws[i].proficient = json_archive["Saving Throws"][i].get<bool>();
		}

		json spells = json_archive["Spells"];

		for (size_t i = 0; i < spells.size(); i++)
		{
			spell_slots.push_back(spells[i].get<SpellSlot>());
		}
		
		CopyJsonStringToString(json_archive, "Languages", languages, multiline_text_length);

		CopyJsonStringToString(json_archive, "Feats", feats, multiline_text_length);

		CopyJsonStringToString(json_archive, "Inventory", inventory, multiline_text_length);

		CopyJsonStringToString(json_archive, "Spellbook", spellbook, extended_multiline_text_length);

		sight = json_archive["Sight"].get<int>();
	}

	void SaveToJson(json& json_archive)
	{
		json_archive["Version"] = version_string;

		json_archive["Name"] = character_name;
		json_archive["Class"] = class_name;
		json_archive["Level"] = character_level;
		json_archive["Background"] = background;
		json_archive["Party"] = party_name;
		json_archive["Race"] = race;
		json_archive["Experience"] = experience;
		json_archive["Alignment"] = alignment;
		json_archive["Inspiration"] = inspiration;
		json_archive["Initiative"] = initiative;

		json_archive["Attributes"]["Strength"] = strength;
		json_archive["Attributes"]["Dexterity"] = dexterity;
		json_archive["Attributes"]["Constitution"] = constitution;
		json_archive["Attributes"]["Intelligence"] = intelligence;
		json_archive["Attributes"]["Wisdom"] = wisdom;
		json_archive["Attributes"]["Charisma"] = charisma;

		json_archive["Passive Perception"] = passive_perception;

		json_archive["Armour Class"] = armour_class;


		json_archive["Speed"] = speed;

		json_archive["Max Hit Points"] = max_hit_points;

		json_archive["Hit Points"] = current_hit_points;

		json_archive["Hit Die"] = hit_die;

		json_archive["Current Hit Dice"] = hit_dice;

		json_archive["Spellcasting Ability"] = spellcasting_ability;

		json_archive["Spell Save DC"] = spell_save_dc;

		json_archive["Spell Attack Bonus"] = spell_attack_bonus;

		
		


		std::array<bool, 18> proficiencies;
		for (int i = 0; i < skills.size(); i++)
		{
			proficiencies[i] = skills[i].proficient;
		}
		json_archive["Proficiencies"] = proficiencies;

		std::array<bool, 6> saves;
		for (size_t i = 0; i < saving_throws.size(); i++)
		{
			saves[i] = saving_throws[i].proficient;
		}
		json_archive["Saving Throws"] = saves;



		json_archive["Spells"] = spell_slots;

		json_archive["Spellbook"] = spellbook;

		json_archive["Languages"] = languages;

		json_archive["Inventory"] = inventory;

		json_archive["Feats"] = feats;

		json_archive["Sight"] = sight;
	}

	

	
	
	char* character_name { nullptr };
	
	char* class_name { nullptr };
	
	char* background { nullptr };
	
	char* party_name{ nullptr };
	
	char* race{ nullptr };
	
	char* experience{ nullptr };

	int character_level { 1 };
	
	int alignment{ 1 };
	
	int proficiency{ 1 };
	
	int inspiration{ 0 };


	int strength{ 10 };
	int dexterity{ 10 };
	int constitution{ 10 };
	int intelligence{ 10 };
	int wisdom{ 10 };
	int charisma{ 10 };


	int passive_perception{ 10 };

	int armour_class{ 10 };

	int initiative{ 0 };

	int speed{ 30 };

	int max_hit_points{ 1 };

	int current_hit_points{ 1 };

	int hit_die{ 1 };

	int hit_dice{ 1 };

	int spellcasting_ability{ 1 };

	int spell_save_dc{ 8 };

	int spell_attack_bonus{ 1 };

	std::vector<char*> strings;

	std::array<const char*, 9> alignment_strings;
	std::array<const char*, 6> ability_short_strings;
	std::array<const char*, 6> die_strings;
	std::array<const char*, 4> sight_strings;
	std::array<SkillEntry, 18> skills;
	std::array<SavingThrow, 6> saving_throws;

	std::vector<SpellSlot> spell_slots;

	char edit_slot_name[normal_text_length]{"Spell Slot"};
	int edit_slot_level{ 1 };

	char* languages{ nullptr };

	char* spellbook{ nullptr };

	char* inventory{ nullptr };

	char* feats{ nullptr };

	int sight{ 0 };
};


int main()
{
	json j;

	std::ifstream settings("settings.json");
	if (settings.good())
	{
		settings >> j;
	}
	settings.close();
	
	int width{1280}, height{ 720 };
	if (j.count("Width"))
	{
		width = j["Width"].get<int>();
	}
	if (j.count("Height"))
	{
		height = j["Height"].get<int>();
	}
	sf::RenderWindow window(sf::VideoMode(width, height), "Character Sheet");

	window.setVerticalSyncEnabled(true);

	ImGui::SFML::Init(window);

	sf::Clock delta_time_clock;

	CharacterSheet character_sheet;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			default:
				break;
			}
		}
		ImGui::SFML::Update(window, delta_time_clock.restart());

		character_sheet.DoGUI(width, height);

		window.clear(sf::Color::White);

		ImGui::SFML::Render();

		window.display();
	}
}