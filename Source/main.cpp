#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nfd.h>

#include <array>
#include <cmath>



constexpr size_t normal_text_length = 32;
constexpr size_t name_text_length = 128;

struct SkillEntry
{
	bool proficient{ false };
	int* base_attribute{ nullptr };
	char base_attribute_string[normal_text_length]{ "" };
	char skill_name[normal_text_length]{ "" };
};

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
		InitString(player_name, name_text_length);
		InitString(race, normal_text_length);
		InitString(experience, normal_text_length);

		alignment_strings = { "Lawful Good", "Neutral Good", "Chaotic Good", "Lawful Neutral", "Neutral", "Chaotic Neutral", "Lawful Evil", "Neutral Evil", "Chaotic Evil" };
		character_level = 1;

		proficiency = 1 + (int)std::ceil(character_level / 4.0);

		for (auto& it : skills)
		{
			it.base_attribute = &strength;
		}
		FillSkillsTable();

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


	void DoGUI()
	{
		
		const int min_level = 1;
		// Equations for proficiency still work for the level 50 variant rule
		const int max_level = 20;

		ImGui::Begin("Character Sheet", nullptr, ImGuiWindowFlags_NoCollapse);
		if (ImGui::Button("Save Character Sheet"))
		{
			// Save Sheet Here
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Character Sheet"))
		{
			// Load Sheet Here
		}


		ImGui::InputText("Character Name", character_name, name_text_length);

		ImGui::SliderInt("Level", &character_level, min_level, max_level);
		
		ImGui::InputText("Class", class_name, normal_text_length);

		ImGui::InputText("Background", background, normal_text_length);

		ImGui::InputText("Player Name", player_name , name_text_length);

		ImGui::InputText("Race", race, normal_text_length);

		ImGui::Combo("Alignment", &alignment, alignment_strings.data(), alignment_strings.size());

		ImGui::InputText("Experience", experience, normal_text_length);

		proficiency = 1 + (int)std::ceil(character_level / 4.0);

		ImGui::Text("Proficiency Bonus [%d]", proficiency);

		ImGui::InputInt("Inspiration", &inspiration, 1, 1);
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
	char* character_name { nullptr };
	
	char* class_name { nullptr };
	
	char* background { nullptr };
	
	char* player_name{ nullptr };
	
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

	std::vector<char*> strings;

	std::array<const char*, 9> alignment_strings;

	std::array<SkillEntry, 18> skills;
};


int main()
{
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Character Sheet");

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

		character_sheet.DoGUI();

		window.clear(sf::Color::White);

		ImGui::SFML::Render();

		window.display();
	}
}