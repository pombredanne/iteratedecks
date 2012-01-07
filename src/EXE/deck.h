// *****************************************
// EvaluateDecks
// Tyrant card game simulator
//
// My kongregate account:
// http://www.kongregate.com/accounts/NETRAT
// 
// Project pages:
// http://code.google.com/p/evaluatedecks
// http://www.kongregate.com/forums/65-tyrant/topics/195043-yet-another-battlesim-evaluate-decks
// *****************************************
//
// this module contains all evaluation related stuff - card and deck classes and their interactions

#include <vector>
#include <set>

#define DEFAULT_DECK_SIZE		10
#define DEFAULT_DECK_RESERVE_SIZE	15 // up to 20? kinda deck max size for structures
#define DEFAULT_HAND_SIZE		3

typedef	unsigned char UCHAR;
typedef	unsigned int UINT;

#include "results.h"

#define CARD_NAME_MAX_LENGTH	50 // must sync it with CARD_NAME_MAX_LENGTH in interface
#define FILENAME_MAX_LENGTH		50 //
#define CARD_ABILITIES_MAX		70 // must sync it with CARD_ABILITIES_MAX in interface

#define FANCY_STATS_COUNT		5

#define CARD_MAX_ID				4000 // size of storage array
#define MISSION_MAX_ID			250  // size of storage array
#define RAID_MAX_ID				20  // size of storage array

#define RARITY_COMMON			0
#define RARITY_UNCOMMON			1
#define RARITY_RARE				2
#define RARITY_UNIQUE			3
#define RARITY_LEGENDARY		4
#define RARITY_STORYCOMMANDER	10

#define FACTION_NONE			0
#define FACTION_IMPERIAL		1
#define FACTION_RAIDER			2
#define FACTION_BLOODTHIRSTY	3
#define FACTION_XENO			4
#define FACTION_RIGHTEOUS		5

char FACTIONS[6][CARD_NAME_MAX_LENGTH] = {0,"Imperial","Raider","Bloodthirsty","Xeno","Righteous"};

//#define TARGETSCOUNT_NONE		0				
#define TARGETSCOUNT_ONE		0				
#define TARGETSCOUNT_ALL		10	

#define TYPE_NONE				0
#define TYPE_COMMANDER			1
#define TYPE_ASSAULT			2
#define TYPE_STRUCTURE			3
#define TYPE_ACTION				4

#define ABILITY_ENABLED			1  // just a helper, no use

#define ACTIVATION_CHAOS		11
#define ACTIVATION_CLEANSE		12
#define ACTIVATION_ENFEEBLE		13
#define ACTIVATION_FREEZE		14
#define ACTIVATION_HEAL			15
#define ACTIVATION_INFUSE		16	// this currently works only for Commander	
#define ACTIVATION_JAM			17
#define ACTIVATION_MIMIC		18
#define ACTIVATION_PROTECT		19
#define ACTIVATION_RALLY		20
#define ACTIVATION_RECHARGE		21
#define ACTIVATION_REPAIR		22
#define ACTIVATION_SHOCK		23
#define ACTIVATION_SIEGE		24
#define ACTIVATION_SPLIT		25
#define ACTIVATION_STRIKE		26
#define ACTIVATION_SUPPLY		27
#define ACTIVATION_WEAKEN		28

#define DEFENSIVE_ARMORED		31
#define DEFENSIVE_COUNTER		32
#define DEFENSIVE_EVADE			33
#define DEFENSIVE_FLYING		34
#define DEFENSIVE_PAYBACK		35
#define DEFENSIVE_REFRESH		36
#define DEFENSIVE_REGENERATE	37
#define DEFENSIVE_TRIBUTE		38
#define DEFENSIVE_WALL			39

#define COMBAT_ANTIAIR			41
#define COMBAT_FEAR				42
#define COMBAT_FLURRY			43
#define COMBAT_PIERCE			44
#define COMBAT_SWIPE			45
#define COMBAT_VALOR			46

#define DMGDEPENDANT_BERSERK	51
#define DMGDEPENDANT_CRUSH		52
#define DMGDEPENDANT_DISEASE	53
#define DMGDEPENDANT_IMMOBILIZE	54
#define DMGDEPENDANT_LEECH		55
#define DMGDEPENDANT_POISON		56
#define DMGDEPENDANT_SIPHON		57 

#define SPECIAL_BACKFIRE		61  // Destroyed - When this is destroyed, deal damage to own Commander.

// SKILLS THAT ARE NOT DEFINED AND NOT WORKING:
#define SPECIAL_FUSION			62	// in this sim only works for ACTIVATION skills of STRUCTURES
#define SPECIAL_AUGMENT			63
#define SPECIAL_MIST			64 // this skill doesn't change anything in autoplay // max number refers to CARD_ABILITIES_MAX
#define SPECIAL_BLIZZARD		65 // this skill doesn't change anything in autoplay // max number refers to CARD_ABILITIES_MAX

#define UNDEFINED_NAME			"UNDEFINED"

using namespace std;

bool bConsoleOutput = false; // this is global just for convinience, should be DEFINE instead, to cleanup the code


#define PROC50	Proc()
const bool PROC50
{
	return (rand()%100 < 50);
}

const unsigned short ID2BASE64(const UINT Id)
{
	_ASSERT(Id < 0xFFF);
#define EncodeBase64(x) (x < 26) ? (x + 'A') : ((x < 52) ? (x + 'a' - 26) : ((x < 62) ? (x + '0' - 52) : ((x == 62) ? ('+') : ('/'))))	
	// please keep in mind that any integer type has swapped hi and lo bytes
	// i have swapped them here so we will have correct 2 byte string in const char* GetID64 function
	return ((EncodeBase64(((Id >> 6) & 63)))/* << 8*/) + ((EncodeBase64((Id & 63))) << 8); // so many baneli... parenthesis!
}
#define BASE64ID	BASE642ID // alias
const UINT BASE642ID(const unsigned short base64)
{
#define DecodeBase64(x) (((x >= 'A') && (x <= 'Z')) ? (x - 'A') : (((x >= 'a') && (x <= 'z')) ? (x - 'a' + 26) : (((x >= '0') && (x <= '9')) ? (x - '0' + 52) : ((x == '+') ? (62) : (63)))))
	// same stuff as with ID2BASE64, hi and lo swapped
	return DecodeBase64((base64 & 0xFF)) + DecodeBase64((base64 >> 8)) * 64;
}

class Card
{
private:
	UINT Id; 
	char Name[CARD_NAME_MAX_LENGTH];
	char Picture[FILENAME_MAX_LENGTH];
	UCHAR Type;
	UCHAR Wait;
	UINT Set;
	UCHAR Faction;
	UCHAR Attack;
	UCHAR Health;
	UCHAR Rarity;
	UCHAR Effects[CARD_ABILITIES_MAX];
	UCHAR TargetCounts[CARD_ABILITIES_MAX];
	UCHAR TargetFactions[CARD_ABILITIES_MAX]; // reserved negative values for faction
#define RESERVE_ABILITIES_COUNT	3
	vector<UCHAR> AbilitiesOrdered;
protected:
	void CopyName(const char *name)
	{
		if (name)
		{
			UCHAR len = strlen(name);
			memcpy(Name,name,len);
			Name[len] = 0;
		}
		else
			memcpy(Name,UNDEFINED_NAME,sizeof(UNDEFINED_NAME)+1);
	}
	void CopyPic(const char *pic)
	{
		if (pic)
		{
			UCHAR len = strlen(pic);
			memcpy(Picture,pic,len);
			Picture[len] = 0;
		}
		else
			memcpy(Picture,UNDEFINED_NAME,sizeof(UNDEFINED_NAME)+1);
	}
public:
	Card()
	{
		Id = 0;
		memset(Name,0,CARD_NAME_MAX_LENGTH);
		memset(Effects,0,CARD_ABILITIES_MAX);
		memset(TargetCounts,0,CARD_ABILITIES_MAX);
		memset(TargetFactions,0,CARD_ABILITIES_MAX);
		//AbilitiesOrdered.reserve(RESERVE_ABILITIES_COUNT);
	}
	Card(const UINT id, const char* name, const char* pic, const UCHAR rarity, const UCHAR type, const UCHAR faction, const UCHAR attack, const UCHAR health, const UCHAR wait, const UINT set)
	{
		Id = id;
		//UINT temp = ID2BASE64(4000);
		//printf("%s -> %d\n",(char*)&temp,BASE64ID(temp));
		//BASE642ID(temp);
		CopyName(name);
		CopyPic(pic);
		Type = type;
		Faction = faction;
		Attack = attack;
		Health = health;
		Wait = wait;
		Rarity = rarity;
		Set = set;
		memset(Effects,0,CARD_ABILITIES_MAX);
		memset(TargetCounts,0,CARD_ABILITIES_MAX);
		memset(TargetFactions,0,CARD_ABILITIES_MAX);
		AbilitiesOrdered.reserve(RESERVE_ABILITIES_COUNT);
	}
	Card(const Card &card)
	{
		Id = card.Id;
		memcpy(Name,card.Name,CARD_NAME_MAX_LENGTH);
		memcpy(Picture,card.Picture,FILENAME_MAX_LENGTH);
		Type = card.Type;
		Faction = card.Faction;
		Attack = card.Attack;
		Health = card.Health;
		Wait = card.Wait;
		Rarity = card.Rarity;
		Set = card.Set;
		memcpy(Effects,card.Effects,CARD_ABILITIES_MAX);
		memcpy(TargetCounts,card.TargetCounts,CARD_ABILITIES_MAX);
		memcpy(TargetFactions,card.TargetFactions,CARD_ABILITIES_MAX);
		AbilitiesOrdered.reserve(RESERVE_ABILITIES_COUNT);
		if (!card.AbilitiesOrdered.empty())
			for (UCHAR i=0;i<card.AbilitiesOrdered.size();i++)
				AbilitiesOrdered.push_back(card.AbilitiesOrdered[i]);
	}
	Card& operator=(const Card &card)
	{
		Id = card.Id;
		memcpy(Name,card.Name,CARD_NAME_MAX_LENGTH);
		memcpy(Picture,card.Picture,FILENAME_MAX_LENGTH);
		Type = card.Type;
		Faction = card.Faction;
		Attack = card.Attack;
		Health = card.Health;
		Wait = card.Wait;
		Rarity = card.Rarity;
		Set = card.Set;
		memcpy(Effects,card.Effects,CARD_ABILITIES_MAX);
		memcpy(TargetCounts,card.TargetCounts,CARD_ABILITIES_MAX);
		memcpy(TargetFactions,card.TargetFactions,CARD_ABILITIES_MAX);
		AbilitiesOrdered.reserve(RESERVE_ABILITIES_COUNT);
		if (!card.AbilitiesOrdered.empty())
			for (UCHAR i=0;i<card.AbilitiesOrdered.size();i++)
				AbilitiesOrdered.push_back(card.AbilitiesOrdered[i]);
		return *this;
	}
	void AddAbility(const UCHAR id, const UCHAR effect, const UCHAR targetcount, const UCHAR targetfaction)
	{
		Effects[id] = effect;
		TargetCounts[id] = targetcount;
		TargetFactions[id] = targetfaction;
		AbilitiesOrdered.push_back(id);
	}
	void AddAbility(const UCHAR id, const UCHAR targetcount, const UCHAR targetfaction)
	{
		Effects[id] = ABILITY_ENABLED;
		TargetCounts[id] = targetcount;
		TargetFactions[id] = targetfaction;
		AbilitiesOrdered.push_back(id);
	}
	void AddAbility(const UCHAR id, const UCHAR effect)
	{
		Effects[id] = effect;
		AbilitiesOrdered.push_back(id);
	}
	void AddAbility(const UCHAR id)
	{
		Effects[id] = ABILITY_ENABLED;
		AbilitiesOrdered.push_back(id);
	}
	void PrintAbilities()
	{
		for (UCHAR i=0;i<CARD_ABILITIES_MAX;i++)
			if (Effects[i] > 0)
				printf("%d ",i);
		printf("\n");
	}
	void Destroy() {	Id = 0;	}
	~Card()	{ Destroy(); }
	const bool IsCard() const { return (Id != 0); }
	const UINT GetId() const { return Id; }
	const char* GetID16(UINT &ID16Storage, bool bLowerCase = false) const
	{
		UCHAR c = Id & 0xF;
		UCHAR baseA = (bLowerCase) ? ('a' - 10) : ('A' - 10); // I thought I told I don't like this style ;)
		char *ptr = (char *)&ID16Storage;
		c = (c < 10) ? (c + '0') : (c + baseA); 
		ptr[2] = c;
		c = (Id >> 4) & 0xF;
		c = (c < 10) ? (c + '0') : (c + baseA);
		ptr[1] = c;
		c = (Id >> 8) & 0xF;
		c = (c < 10) ? (c + '0') : (c + baseA);
		ptr[0] = c;
		ptr[3] = 0;
		return (const char*)ptr;
	}
	const unsigned short GetID64() const
	{
		return ID2BASE64(Id);
	}
	const char* GetID64(UINT &ID64Storage) const
	{
		//ID64Storage = 0; // is it nessesary?
		ID64Storage = GetID64();
		return (char *)&ID64Storage;
	}
	const UCHAR GetAttack() const	{	return Attack;	}
	const UCHAR GetHealth() const	{	return Health;	}
	const UCHAR GetWait() const		{	return Wait;	}
	const UCHAR GetType() const		{	return Type;	}
	const UCHAR GetSet() const		{	return Set;		}
	const UCHAR GetRarity() const { return Rarity; }
	const UCHAR GetFaction() const { return Faction; }
	const UCHAR GetAbility(const UCHAR id) const { return Effects[id]; }
	const UCHAR GetAbilitiesCount() const { return AbilitiesOrdered.size(); }
	const UCHAR GetAbilityInOrder(const UCHAR order) const
	{
		_ASSERT(AbilitiesOrdered.size() > order);
		if (AbilitiesOrdered.size() <= order)
			return 0;
		else
			return AbilitiesOrdered[order];
	}
	const UCHAR GetTargetCount(const UCHAR id) const { return TargetCounts[id]; }
	const UCHAR GetTargetFaction(const UCHAR id) const { return TargetFactions[id]; }
	const char *GetName() const { return Name; }
	const char *GetPicture() const { return Picture; }
};
class PlayedCard
{
typedef vector<PlayedCard> VCARDS;
typedef vector<PlayedCard*> PVCARDS;
private:
	// constant
	const Card *OriginalCard;
	// temporary
	UCHAR Attack;
	UCHAR Health;
	UCHAR Wait;
	UCHAR Faction; // this is for Infuse
	UCHAR Effects[CARD_ABILITIES_MAX];
	bool bPlayed;
	bool bActivated;
public:
	// fancy stats
	UINT fsDmgDealt; // attack, counter, strike, poison is not here(hard to track the source of poison)
	UINT fsDmgMitigated; // obvious - before it died, should we include overkill?
	UINT fsAvoided; // evade, armor, flying, protect - absorbed damage, this always IGNORES protect and ignores armor if it's a flying miss
	UINT fsHealed; // supply, heal, leech, regenerate
	UINT fsSpecial; // enfeeble, protect, weaken, rally, poison(when applied)
	// skill proc counter
	UCHAR *SkillProcBuffer;
public:
	void DecWait() { if (Wait) Wait--; }
	void IncWait() { if (Wait) Wait++; }  // this is only for tournaments
	const char *GetName() const { return OriginalCard->GetName(); }
	void PrintDesc()
	{
		if (IsDefined())
		{
			if (OriginalCard->GetAttack() && OriginalCard->GetWait())
				printf("%s - %X [%d/%d|%d]",OriginalCard->GetName(),this,GetAttack(),Health,Wait);
			else
				if (OriginalCard->GetWait())
					printf("%s - %X [%d|%d]",OriginalCard->GetName(),this,Health,Wait);
				else
					printf("%s - %X [%d]",OriginalCard->GetName(),this,Health);
			if (Effects[ACTIVATION_JAM])
				printf(" Jammed");
			if (Effects[ACTIVATION_FREEZE])
				printf(" Frozen");
			if (Effects[DMGDEPENDANT_IMMOBILIZE])
				printf(" Immobilized");
			if (Effects[DMGDEPENDANT_DISEASE])
				printf(" Diseased");
			if (Effects[ACTIVATION_CHAOS])
				printf(" Chaosed");
		}
	}
	void SetCardSkillProcBuffer(UCHAR *_SkillProcBuffer)
	{
		SkillProcBuffer = _SkillProcBuffer;
	}
	void CardSkillProc(UCHAR aid)
	{
		if (SkillProcBuffer)
			SkillProcBuffer[aid]++;
	}
	const bool BeginTurn()
	{
		const bool bDoBegin = (Health && (!Effects[ACTIVATION_JAM]) && (!Effects[ACTIVATION_FREEZE]) && (!Wait));
		if (bDoBegin && (!bActivated))
			bActivated = true;
		return bDoBegin;
	}
	void ProcessPoison()
	{
		if (IsAlive() && (Effects[DMGDEPENDANT_POISON]))
			SufferDmg(Effects[DMGDEPENDANT_POISON]);
	}
	const UCHAR GetShield() const
	{
		return Effects[ACTIVATION_PROTECT];
	}
	void ResetShield()
	{
		Effects[ACTIVATION_PROTECT] = 0;
	}
	void Refresh()
	{
		fsHealed += (OriginalCard->GetHealth() - Health);
		if (OriginalCard->GetHealth() != Health)
			SkillProcBuffer[DEFENSIVE_REFRESH]++;
		Health = OriginalCard->GetHealth();
	}
	void ClearEnfeeble()
	{
		Effects[ACTIVATION_ENFEEBLE] = 0;
	}
	void RemoveDebuffs()
	{
/*
by Moraku:
Chaos: Removed after owner ends his turn.
Disease: Never removed (unless cleansed).
Enfeeble: Removed after either player ends his turn.
Fusion: Never removed (unless less than 3 Fusion cards active).
Immobilize: Removed after owner ends his turn.
Infusion: Never removed.
Jam: Removed after owner ends his turn.
Poison: Never removed (unless cleansed).
Protect: Removed before owner begins his turn.
Rally: Removed after owner ends his turn.
Weaken: Removed after owner ends his turn.
Valor: Removed after owner ends his turn.
*/
		Effects[ACTIVATION_JAM] = 0;
		Effects[ACTIVATION_FREEZE] = 0; // is it removed at the end of turn?
		Effects[DMGDEPENDANT_IMMOBILIZE] = 0;
		Effects[ACTIVATION_CHAOS] = 0;
		// really?
		Effects[ACTIVATION_RALLY] = 0;
		Effects[ACTIVATION_WEAKEN] = 0;
	}
	void Cleanse()
	{
		Effects[DMGDEPENDANT_POISON] = 0;
		Effects[DMGDEPENDANT_DISEASE] = 0;
		Effects[ACTIVATION_JAM] = 0;		
		if (bActivated)  // this is bullshit!
			Effects[ACTIVATION_FREEZE] = 0;
		Effects[DMGDEPENDANT_IMMOBILIZE] = 0;
		Effects[ACTIVATION_ENFEEBLE] = 0;
		Effects[ACTIVATION_CHAOS] = 0;
	}
	bool IsCleanseTarget()
	{
		// Poison, Disease, Jam, Immobilize, Enfeeble, Chaos. (Does not remove Weaken.)
		return (Effects[DMGDEPENDANT_POISON] || 
				Effects[DMGDEPENDANT_DISEASE] || 
				Effects[ACTIVATION_JAM] ||
				(Effects[ACTIVATION_FREEZE] && bActivated) ||
				Effects[DMGDEPENDANT_IMMOBILIZE] || 
				Effects[ACTIVATION_ENFEEBLE] ||
				Effects[ACTIVATION_CHAOS]);
	}
	void EndTurn()
	{
		Played(); // for rally
		if ((Wait > 0) && (!Effects[ACTIVATION_FREEZE]))
			DecWait();
	}
	const UCHAR GetAbilitiesCount() const { return OriginalCard->GetAbilitiesCount(); }
	const UCHAR GetAbilityInOrder(const UCHAR order) const { return OriginalCard->GetAbilityInOrder(order); }
	void Infuse(const UCHAR setfaction)
	{
		//OriginalCard.Infuse(setfaction);
		Faction = setfaction;
		SkillProcBuffer[ACTIVATION_INFUSE]++;
	}
	const UCHAR SufferDmg(const UCHAR Dmg, const UCHAR Pierce = 0, UCHAR *actualdamagedealt = 0, UCHAR *SkillProcBuffer = 0)
	{
		_ASSERT(OriginalCard);
// Regeneration happens before the additional strikes from Flurry.
// Regenerating does not prevent Crush damage	
		UCHAR dmg = Dmg;
		UCHAR shield = GetEffect(ACTIVATION_PROTECT);
		if (shield > 0)
		{
			if (Pierce >= shield)
				dmg = Dmg;
			else
			{
				if (Pierce+Dmg >= shield)
					dmg = Pierce + Dmg - shield;
				else
					dmg = 0;
			}
		}
		if (dmg >= Health)
		{
			UCHAR dealt = Health;
			if ((!IsDiseased()) && (OriginalCard->GetAbility(DEFENSIVE_REGENERATE))&&(PROC50))
			{
				Health = OriginalCard->GetAbility(DEFENSIVE_REGENERATE);
				fsHealed += OriginalCard->GetAbility(DEFENSIVE_REGENERATE);
				CardSkillProc(DEFENSIVE_REGENERATE);
				if (bConsoleOutput)
				{
					PrintDesc();
					printf(" regenerated % health\n",Health);
				}
			}
			else
			{
				Health = 0;
				if (bConsoleOutput)
				{
					PrintDesc();
					printf(" died!\n");
				}
			}
			if (actualdamagedealt) // siphon and leech are kinda bugged - overkill damage counts as full attack damage even if card has 1 hp left, therefore this workaround
			{
				*actualdamagedealt = dealt;
				return dmg;
			}
			// crush damage will be dealt even if the defending unit Regenerates
			return dealt;// note that CRUSH & BACKFIRE are processed elsewhere
		}
		else
		{
			Health -= dmg;
			if (bConsoleOutput)
			{
				PrintDesc();
				printf(" suffered %d damage\n",dmg);
			}
		}
		fsDmgMitigated += dmg;
		return dmg;
	}
	bool HitCommander(const UCHAR Dmg, PlayedCard &Src, VCARDS &Structures, bool bCanBeCountered = true)
	{
		_ASSERT(GetType() == TYPE_COMMANDER); // double check for debug
		_ASSERT(Dmg); // 0 dmg is pointless and indicates an error
		// find a wall to break it ;)
		for (VCARDS::iterator vi = Structures.begin();vi!=Structures.end();vi++)
			if (vi->GetAbility(DEFENSIVE_WALL) && vi->IsAlive())
			{
				vi->CardSkillProc(DEFENSIVE_WALL);
				// walls can counter and regenerate
				vi->SufferDmg(Dmg); // regenerate
				if (vi->GetAbility(DEFENSIVE_COUNTER) && bCanBeCountered) // counter, dmg from crush can't be countered
				{
					vi->CardSkillProc(DEFENSIVE_COUNTER);
					vi->fsDmgDealt += vi->GetAbility(DEFENSIVE_COUNTER) + Src.GetEffect(ACTIVATION_ENFEEBLE);
					Src.SufferDmg(vi->GetAbility(DEFENSIVE_COUNTER) + Src.GetEffect(ACTIVATION_ENFEEBLE)); // counter dmg is enhanced by enfeeble
				}
				return false;
			}
		// no walls found then hit commander
		// ugly - counter procs before commander takes dmg, but whatever
		if (GetAbility(DEFENSIVE_COUNTER) && bCanBeCountered) // commander can counter aswell
		{
			CardSkillProc(DEFENSIVE_COUNTER);
			Src.SufferDmg(GetAbility(DEFENSIVE_COUNTER) + Src.GetEffect(ACTIVATION_ENFEEBLE)); // counter dmg is enhanced by enfeeble
		}
		return (SufferDmg(Dmg) > 0);
	}
	UCHAR StrikeDmg(const UCHAR Dmg) // returns dealt dmg
	{
		_ASSERT(Dmg); // 0 dmg is pointless and indicates an error
		//printf("%s %d <- %d\n",GetName(),GetHealth(),Dmg);
		return SufferDmg(Dmg + Effects[ACTIVATION_ENFEEBLE]);
	}
	const bool IsAlive() const
	{
		return ((OriginalCard) &&/* THIS IS CRAP! */(Health != 0)); //(Attack||Health||Wait);
	}
	const bool IsDefined() const
	{
		return (OriginalCard && (Attack||Health||Wait||(GetType() == TYPE_COMMANDER)||(GetType() == TYPE_ACTION)));
	}
	const UCHAR GetRarity() const
	{
		return OriginalCard->GetRarity();
	}
	bool operator==(const PlayedCard &C) const
	{
		if (OriginalCard != C.OriginalCard) // better to compare ID's, but this should also work
			return false;
		if (Wait != C.Wait)
			return false;
		if (Attack != C.Attack)
			return false;
		if (Health != C.Health)
			return false;
		if (bPlayed != C.bPlayed)
			return false;
		if (bActivated != C.bActivated)
			return false;		
		if (Faction != C.Faction)
			return false;
		return (!memcmp(Effects,C.Effects,CARD_ABILITIES_MAX * sizeof(UCHAR)));
	}
	bool operator!=(const PlayedCard &C) const
	{
		if (OriginalCard != C.OriginalCard) // better to compare ID's, but this should also work
			return true;
		if (Wait != C.Wait)
			return true;
		if (Attack != C.Attack)
			return true;
		if (Health != C.Health)
			return true;
		if (bPlayed != C.bPlayed)
			return true;
		if (bActivated != C.bActivated)
			return true;
		if (Faction != C.Faction)
			return true;
		return (memcmp(Effects,C.Effects,CARD_ABILITIES_MAX * sizeof(UCHAR)) != 0);
	}
	bool operator<(const PlayedCard &C) const
	{
		if (OriginalCard != C.OriginalCard) // better to compare ID's, but this should also work
			return (OriginalCard < C.OriginalCard);
		if (Wait != C.Wait)
			return (Wait < C.Wait);
		if (Attack != C.Attack)
			return (Attack < C.Attack);
		if (Health != C.Health)
			return (Health < C.Health);
		if (bPlayed != C.bPlayed)
			return (bPlayed < C.bPlayed);
		if (bActivated != C.bActivated)
			return (bActivated < C.bActivated);		
		if (Faction != C.Faction)
			return (Faction < C.Faction);
		int mr = memcmp(Effects,C.Effects,CARD_ABILITIES_MAX * sizeof(UCHAR)) < 0;
		return (mr < 0) && (mr != 0);
	}
	PlayedCard& operator=(const Card *card)
	{
		_ASSERT(card);
		OriginalCard = card;
		Attack = card->GetAttack();
		Health = card->GetHealth();
		Wait = card->GetWait();
		Faction = card->GetFaction();
		bPlayed = false;
		bActivated = false;
		memset(Effects,0,CARD_ABILITIES_MAX);
		fsDmgDealt = 0;
		fsDmgMitigated = 0;
		fsAvoided = 0;
		fsHealed = 0;
		fsSpecial = 0;
		SkillProcBuffer = 0;
		return *this;
	}
	PlayedCard(const Card *card) 
	{
		_ASSERT(card);
		OriginalCard = card;
		Attack = card->GetAttack();
		Health = card->GetHealth();
		Wait = card->GetWait();
		Faction = card->GetFaction();
		bPlayed = false;
		bActivated = false;
		memset(Effects,0,CARD_ABILITIES_MAX);
		fsDmgDealt = 0;
		fsDmgMitigated = 0;
		fsAvoided = 0;
		fsHealed = 0;
		fsSpecial = 0;
		SkillProcBuffer = 0;
	}
	const UINT GetId() const { return OriginalCard->GetId(); }
	const UCHAR GetNativeAttack() const
	{
		return Attack;
	}
	const UCHAR GetAttack() const
	{
		char atk = Attack - Effects[ACTIVATION_WEAKEN] + Effects[ACTIVATION_RALLY];
		//printf("[%X] %d = %d - %d + %d\n",this,atk,Attack,Effects[ACTIVATION_WEAKEN],Effects[ACTIVATION_RALLY]);
		if (atk > 0)
			return (UCHAR)atk;
		else
			return 0;
	}
	const UCHAR GetHealth() const { return Health; }
	const UCHAR GetMaxHealth() const { return OriginalCard->GetHealth(); }
	const UCHAR GetFaction() const { return Faction; }
	const UCHAR GetWait() const { return Wait; }
	const UCHAR GetType() const	
	{
		// this is crap ! - must remove and check the code
		if (!OriginalCard)
			return TYPE_NONE;
		return OriginalCard->GetType();	
	}
	const UCHAR GetEffect(const UCHAR id) const { return Effects[id]; }
	const UCHAR GetAbility(const UCHAR id) const
	{
		// this is crap ! - must remove and check the code
		if (!OriginalCard)
			return 0;
		return OriginalCard->GetAbility(id); 
	}
	const UCHAR GetTargetCount(const UCHAR id) const { return OriginalCard->GetTargetCount(id); }
	const UCHAR GetTargetFaction(const UCHAR id) const
	{
		// this is for infuse
		// if card was infused, we gotta force target faction it was infused into
		if ((Faction != OriginalCard->GetFaction()) && (OriginalCard->GetTargetFaction(id) != FACTION_NONE))// check if card was infused and targetfaction is determined
			return Faction; // force faction it was infused into
		else
			return OriginalCard->GetTargetFaction(id);
	}
	const bool GetPlayed() const { return bPlayed; }
	void Played() { bPlayed = true; }
	void ResetPlayedFlag() { bPlayed = false; }
	void SetAttack(const UCHAR attack) { Attack = attack; }
	void SetHealth(const UCHAR health) { Health = health; }
	void SetEffect(const UCHAR id, const UCHAR value) { Effects[id] = value; }
	void Rally(const UCHAR amount)
	{
		Effects[ACTIVATION_RALLY] += amount;
		//Attack += amount;
	}
	UCHAR Weaken(const UCHAR amount)
	{
		Effects[ACTIVATION_WEAKEN] += amount;
		//if (amount > Attack) Attack = 0; else Attack -= amount;
		return amount; // this is correct and incorrect at the same time ;(
	}
	void Berserk(const UCHAR amount)
	{
		Attack += amount;
	}
	void Protect(const UCHAR amount)
	{
		//if (amount > Effects[ACTIVATION_PROTECT])
		Effects[ACTIVATION_PROTECT] += amount;
	}
	const bool IsDiseased() const	{	return Effects[DMGDEPENDANT_DISEASE] > 0; }
	UCHAR Heal(UCHAR amount)
	{
		_ASSERT(!IsDiseased()); // disallowed
		if (IsDiseased()) return 0;
		if (Health + amount >  OriginalCard->GetHealth())
		{
			amount = (OriginalCard->GetHealth() - Health);
			Health =  OriginalCard->GetHealth();
		}
		else
			Health += amount;
		return amount;
	}
	const Card *GetOriginalCard() const { return OriginalCard; }
//protected:
	PlayedCard() 
	{
		Attack = 0;
		Health = 0;
		Wait = 0;
		Faction = 0;
		bPlayed = false;
		bActivated = false;
		OriginalCard = 0;
		memset(Effects,0,CARD_ABILITIES_MAX);
		fsDmgDealt = 0;
		fsDmgMitigated = 0;
		fsAvoided = 0;
		fsHealed = 0;
		fsSpecial = 0;
		SkillProcBuffer = 0;
	}
};
struct REQUIREMENT
{
	UCHAR SkillID;
	UCHAR Procs;
	REQUIREMENT() { SkillID = 0; };
};
typedef vector<PlayedCard> VCARDS;
typedef vector<PlayedCard*> PVCARDS;
typedef multiset<UINT> MSID;
class ActiveDeck
{
public:
	PlayedCard Commander;
	VCARDS Deck;
	VCARDS Units;
	VCARDS Structures;
	VCARDS Actions;
	//
	bool bOrderMatters;
	MSID Hand;
	bool bDelayFirstCard; // tournament-like
	//
	const UCHAR *CSIndex;
	RESULT_BY_CARD *CSResult;
	//
	UCHAR SkillProcs[CARD_ABILITIES_MAX];
	//
	UINT CardPicks[DEFAULT_DECK_RESERVE_SIZE];
	//
	UINT DamageToCommander; // for points calculation - damage dealt to ENEMY commander
private:
	void Reserve()
	{
		Deck.reserve(DEFAULT_DECK_RESERVE_SIZE);
		Units.reserve(DEFAULT_DECK_RESERVE_SIZE);
		Structures.reserve(DEFAULT_DECK_RESERVE_SIZE);
		Actions.reserve(DEFAULT_DECK_RESERVE_SIZE);
	}
	UCHAR GetAliveUnitsCount()
	{
		UCHAR c = 0;
		for (UCHAR i=0;i<Units.size();i++)
			if (Units[i].IsAlive())
				c++;
		return c;
	}
	void Attack(UCHAR index, ActiveDeck &Def)
	{
#define SRC	Units[index]
// does anyone know if VALOR procs on commander? imagine combo of valor+flurry or valor+fear
// ok let's assume it does
#define VALOR_HITS_COMMANDER	true
		//printf("%s %d\n",SRC.GetName(),SRC.GetHealth());
		_ASSERT(SRC.IsDefined() && SRC.IsAlive()); // baby, don't hurt me, no more
		if (bConsoleOutput)
		{
			SRC.PrintDesc();
			printf(" attacks\n");
		}
		UCHAR iflurry = (SRC.GetAbility(COMBAT_FLURRY) && PROC50) ? (SRC.GetAbility(COMBAT_FLURRY)+1) : 1; // coding like a boss :) don't like this style
		if (iflurry > 1)
			SkillProcs[COMBAT_FLURRY]++;
		if ((index >= (UCHAR)Def.Units.size()) || (!Def.Units[index].IsAlive()) || (SRC.GetAbility(COMBAT_FEAR)))
		{
			// Deal DMG To Commander BUT STILL PROC50 FLURRY and PROBABLY VALOR
			UCHAR valor = (VALOR_HITS_COMMANDER && SRC.GetAbility(COMBAT_VALOR) && (GetAliveUnitsCount() < Def.GetAliveUnitsCount())) ? SRC.GetAbility(COMBAT_VALOR) : 0;
			for (UCHAR i=0;i<iflurry;i++)
			{
				if (valor)
					SkillProcs[COMBAT_VALOR]++;
				if (Def.Commander.IsAlive())
					DamageToCommander += SRC.GetAttack()+valor;
				if (SRC.GetAbility(COMBAT_FEAR) && (Def.Units.size() > index) && Def.Units[index].IsAlive())
					SkillProcs[COMBAT_FEAR]++;
				Def.Commander.HitCommander(SRC.GetAttack()+valor,SRC,Def.Structures);
				SRC.fsDmgDealt += SRC.GetAttack()+valor;
				// can go berserk after hitting commander too
				if ((SRC.GetAttack()+valor > 0) && SRC.GetAbility(DMGDEPENDANT_BERSERK))
				{
					SRC.Berserk(SRC.GetAbility(DMGDEPENDANT_BERSERK));
					SkillProcs[DMGDEPENDANT_BERSERK]++;
				}
			}
			return; // and thats it!!!
		}
		else
		{
			// oh noes! that wasn't all
			PlayedCard *targets[3];
			// flurry + swipe ...
			UCHAR swipe = (SRC.GetAbility(COMBAT_SWIPE)) ? 3 : 1;
			if (swipe > 1)
			{
				if ((index > 0) && (Def.Units[index-1].IsAlive()))
					targets[0] = &Def.Units[index-1];
				else
					targets[0] = 0;
				targets[1] = &Def.Units[index];
				_ASSERT(targets[1]); // this is aligned to SRC and must be present
				if ((index+1 < (UCHAR)Def.Units.size()) && (Def.Units[index+1].IsAlive()))
					targets[2] = &Def.Units[index+1];
				else
					targets[2] = 0;
			}
			else
				targets[0] = &Def.Units[index];
			// flurry
			for (UCHAR iatk=0;iatk<iflurry;iatk++)
			{
				bool bGoBerserk = false;
				UCHAR iSwiped = 0;
				// swipe
				for (UCHAR s=0;s<swipe;s++)
				{
					if (
						((!s) || (s == 2)) &&
						((!targets[s]) || ( (!targets[s]->IsAlive()) && ((s != 1) && (swipe != 1)) ))
						) // empty slot to the left or right, swipe doesnt proc, if swipe == 1 then targets[0] can't be null
						continue;
					// if target dies during flurry and slot(s == 1) is aligned to SRC, we deal dmg to commander
					if ((!targets[s]->IsAlive()) && ((swipe == 1) || (s == 1)))
					{
						UCHAR valor = (VALOR_HITS_COMMANDER && SRC.GetAbility(COMBAT_VALOR) && (GetAliveUnitsCount() < Def.GetAliveUnitsCount())) ? SRC.GetAbility(COMBAT_VALOR) : 0;
						if (valor > 0)
							SkillProcs[COMBAT_VALOR]++;
						if (Def.Commander.IsAlive())
							DamageToCommander += SRC.GetAttack()+valor;
						Def.Commander.HitCommander(SRC.GetAttack()+valor,SRC,Def.Structures);
						SRC.fsDmgDealt += SRC.GetAttack()+valor;
						// can go berserk after hitting commander too
						if ((SRC.GetAttack()+valor > 0) && SRC.GetAbility(DMGDEPENDANT_BERSERK))
						{
							SRC.Berserk(SRC.GetAbility(DMGDEPENDANT_BERSERK));
							SkillProcs[DMGDEPENDANT_BERSERK]++;
						}
						// might want to add here check:
						// if (!Def.Commander.IsAlive()) return;
						continue;
					}
					iSwiped++;
					_ASSERT(targets[s]->IsAlive()); // must be alive here
					// actual attack
					// must check valor before every attack
					UCHAR valor = (SRC.GetAbility(COMBAT_VALOR) && (GetAliveUnitsCount() < Def.GetAliveUnitsCount())) ? SRC.GetAbility(COMBAT_VALOR) : 0;
					if (valor > 0)
						SkillProcs[COMBAT_VALOR]++;
					// attacking flyer
					UCHAR antiair = SRC.GetAbility(COMBAT_ANTIAIR);
					if (targets[s]->GetAbility(DEFENSIVE_FLYING))
					{
						if ((!antiair) && (!SRC.GetAbility(DEFENSIVE_FLYING)) && PROC50) // missed
						{
							targets[s]->fsAvoided += SRC.GetAttack() + valor + targets[s]->GetEffect(ACTIVATION_ENFEEBLE); // note that this IGNORES armor and protect
							Def.SkillProcs[DEFENSIVE_FLYING]++;
							continue;
						}
					}
					else antiair = 0; // has no effect if target is not flying
					if (antiair > 0)
						SkillProcs[COMBAT_ANTIAIR]++;
					// enfeeble is taken into account before armor
					UCHAR enfeeble = targets[s]->GetEffect(ACTIVATION_ENFEEBLE);
					// now armor & pierce
					UCHAR dmg = SRC.GetAttack() + valor + antiair + enfeeble;
					UCHAR armor = targets[s]->GetAbility(DEFENSIVE_ARMORED);
					UCHAR pierce = SRC.GetAbility(COMBAT_PIERCE);
					bool bPierce = false;
					if (armor)
					{
						Def.SkillProcs[DEFENSIVE_ARMORED]++;
						if (pierce > 0)
							bPierce = true;
						if (pierce >= armor)
						{
							armor = 0;
							pierce -= armor; // this is for shield
						}
						else
						{
							armor -= pierce;
							pierce = 0; // this is for shield
						}
						// substract armor from dmg
						if (armor >= dmg)
						{
							targets[s]->fsAvoided += dmg;
							dmg = 0;
						}
						else
						{
							targets[s]->fsAvoided += armor;
							dmg -= armor; 
						}
					}
					// now we actually deal dmg
					//printf("%s %d = %d => %s %d\n",SRC.GetName(),SRC.GetHealth(),dmg,targets[s]->GetName(),targets[s]->GetHealth());
					if (dmg)
					{
						UCHAR actualdamagedealt = 0;
						bPierce = bPierce || (targets[s]->GetShield() && pierce);
						dmg = targets[s]->SufferDmg(dmg, pierce,&actualdamagedealt);
						SRC.fsDmgDealt += actualdamagedealt;
					}
					if (bPierce)
						SkillProcs[COMBAT_PIERCE]++;
					// and now dmg dependant effects
					if (!targets[s]->IsAlive()) // target just died
					{
						// afaik it ignores walls
						if (targets[s]->GetAbility(SPECIAL_BACKFIRE))
						{
							Def.Commander.SufferDmg(targets[s]->GetAbility(SPECIAL_BACKFIRE));
							DamageToCommander += SRC.GetAbility(DMGDEPENDANT_CRUSH);
							Def.SkillProcs[SPECIAL_BACKFIRE]++;
						}
						// crush
						if (SRC.GetAbility(DMGDEPENDANT_CRUSH))
						{
							if (Def.Commander.IsAlive())
								DamageToCommander += SRC.GetAbility(DMGDEPENDANT_CRUSH);
							Def.Commander.HitCommander(SRC.GetAbility(DMGDEPENDANT_CRUSH),SRC,Def.Structures,false);
							SkillProcs[DMGDEPENDANT_CRUSH]++;
						}
					}
					// counter
					if ((dmg > 0) && targets[s]->GetAbility(DEFENSIVE_COUNTER))
					{
						targets[s]->fsDmgDealt += SRC.SufferDmg(targets[s]->GetAbility(DEFENSIVE_COUNTER) + SRC.GetEffect(ACTIVATION_ENFEEBLE)); // counter dmg is enhanced by enfeeble
						Def.SkillProcs[DEFENSIVE_COUNTER]++;
					}
					// berserk
					if ((dmg > 0) && SRC.GetAbility(DMGDEPENDANT_BERSERK))
						bGoBerserk = true;
					// if target is dead, we dont need to process this effects
					if (targets[s]->IsAlive() && (dmg > 0))
					{
						// immobilize
						if (SRC.GetAbility(DMGDEPENDANT_IMMOBILIZE) && PROC50)
						{
							targets[s]->SetEffect(DMGDEPENDANT_IMMOBILIZE,SRC.GetAbility(DMGDEPENDANT_IMMOBILIZE));
							SRC.fsSpecial++; // is it good?
							SkillProcs[DMGDEPENDANT_IMMOBILIZE]++;
						}
						// disease
						if (SRC.GetAbility(DMGDEPENDANT_DISEASE))
						{
							targets[s]->SetEffect(DMGDEPENDANT_DISEASE,SRC.GetAbility(DMGDEPENDANT_DISEASE));
							SRC.fsSpecial++; // is it good?
							SkillProcs[DMGDEPENDANT_DISEASE]++;
						}
						// poison
						if (SRC.GetAbility(DMGDEPENDANT_POISON))
							if (targets[s]->GetEffect(DMGDEPENDANT_POISON) < SRC.GetAbility(DMGDEPENDANT_POISON)) // overflow
							{
								targets[s]->SetEffect(DMGDEPENDANT_POISON,SRC.GetAbility(DMGDEPENDANT_POISON));
								SRC.fsSpecial += SRC.GetAbility(DMGDEPENDANT_POISON); 
								SkillProcs[DMGDEPENDANT_POISON]++;
							}
					}
					// leech
					if (SRC.IsAlive() && SRC.GetAbility(DMGDEPENDANT_LEECH))
					{
						UCHAR leech = (SRC.GetAbility(DMGDEPENDANT_LEECH) < dmg) ? SRC.GetAbility(DMGDEPENDANT_LEECH) : dmg;
						if (leech && (!SRC.IsDiseased()))
						{
							leech = SRC.Heal(leech);
							SRC.fsHealed += leech;
							if (leech > 0)
								SkillProcs[DMGDEPENDANT_LEECH]++;
						}
					}
					// siphon
					if (SRC.GetAbility(DMGDEPENDANT_SIPHON))
					{
						UCHAR siphon = (SRC.GetAbility(DMGDEPENDANT_SIPHON) < dmg) ? SRC.GetAbility(DMGDEPENDANT_SIPHON) : dmg;
						if (siphon)
						{
							siphon = Commander.Heal(siphon);
							SRC.fsHealed += siphon;
							if (siphon > 0)
								SkillProcs[DMGDEPENDANT_SIPHON]++;
						}
					}
					if (bGoBerserk)
					{
						SRC.Berserk(SRC.GetAbility(DMGDEPENDANT_BERSERK));
						if (SRC.GetAbility(DMGDEPENDANT_BERSERK))
							SkillProcs[DMGDEPENDANT_BERSERK]++;
					}

					if (!SRC.IsAlive()) // died from counter? during swipe
						break;
				}
				if (iSwiped > 1)
					SkillProcs[COMBAT_SWIPE]++;
				if (!SRC.IsAlive()) // died from counter? during flurry
					break;
			}
		}
#undef SRC
	}
public:
	ActiveDeck() 
	{
		bOrderMatters = false; 
		bDelayFirstCard = false; 
		CSIndex = 0; 
		CSResult = 0; 
		DamageToCommander = 0; 
		memset(SkillProcs,0,CARD_ABILITIES_MAX*sizeof(UCHAR)); 
		memset(CardPicks,0,DEFAULT_DECK_RESERVE_SIZE*sizeof(UINT));
	}
	~ActiveDeck() { Deck.clear(); Units.clear(); Structures.clear(); Actions.clear(); }
public:
#define REQ_MAX_SIZE			5
	bool CheckRequirements(const REQUIREMENT *Reqs)
	{
		if (!Reqs) return true;
		for (UCHAR i=0;(i<REQ_MAX_SIZE) && (Reqs[i].SkillID);i++)
			if (SkillProcs[Reqs[i].SkillID] < Reqs[i].Procs)
				return false;
		return true;
	}
	void SetFancyStatsBuffer(const UCHAR *resindex, RESULT_BY_CARD *res)
	{
		CSIndex = resindex;
		CSResult = res;
	}
	UCHAR GetCountInDeck(UINT Id)
	{
		if (Commander.GetId() == Id)
			return 1;
		UCHAR c = 0;
		for (VCARDS::iterator vi = Deck.begin(); vi != Deck.end(); vi++)
			if (vi->GetId() == Id)
				c++;
		return c;
	}
	// please note, contructors don't clean up storages, must do it manually and beforehand, even copy constructor
	ActiveDeck(const char *HashBase64, const Card *pCDB)
	{
		_ASSERT(pCDB);
		_ASSERT(HashBase64);
		CSIndex = 0;
		CSResult = 0;
		DamageToCommander = 0;
		bOrderMatters = false; 
		bDelayFirstCard = false;
		unsigned short tid = 0, lastid = 0;
		memset(SkillProcs,0,CARD_ABILITIES_MAX*sizeof(UCHAR));
		memset(CardPicks,0,DEFAULT_DECK_RESERVE_SIZE*sizeof(UINT));
		//
		size_t len = strlen(HashBase64);
		_ASSERT(!(len & 1)); // bytes should go in pairs
		len = len >> 1; // div 2
		Deck.reserve(DEFAULT_DECK_RESERVE_SIZE);
		for (UCHAR i = 0; i < len; i++)
		{
			tid = BASE64ID((HashBase64[i << 1] << 8) + HashBase64[(i << 1) + 1]);
			if (!i)
			{
				_ASSERT(tid < CARD_MAX_ID);
				_ASSERT((tid >= 1000) && (tid < 2000)); // commander Id boundaries
				Commander = PlayedCard(&pCDB[tid]);
				Commander.SetCardSkillProcBuffer(SkillProcs);
			}
			else
			{
				_ASSERT(i || (tid < CARD_MAX_ID)); // commander card can't be encoded with RLE
				if (tid < CARD_MAX_ID)
				{
					Deck.push_back(&pCDB[tid]);
					lastid = tid;
				}
				else
				{
					for (UINT k = CARD_MAX_ID+1; k < tid; k++) // decode RLE, +1 because we already added one card
						Deck.push_back(&pCDB[lastid]);
				}					
			}
		}
	}
	ActiveDeck(const Card *Cmd) 
	{ 
		bOrderMatters = false; 
		bDelayFirstCard = false; 
		CSIndex = 0; 
		CSResult = 0;
		DamageToCommander = 0; 
		Commander = PlayedCard(Cmd); 
		Commander.SetCardSkillProcBuffer(SkillProcs); 
		Deck.reserve(DEFAULT_DECK_RESERVE_SIZE); 
		memset(SkillProcs,0,CARD_ABILITIES_MAX*sizeof(UCHAR));
		memset(CardPicks,0,DEFAULT_DECK_RESERVE_SIZE*sizeof(UINT));
	};
	ActiveDeck(const ActiveDeck &D) // need copy constructor
	{
		Commander = D.Commander;
		Commander.SetCardSkillProcBuffer(SkillProcs);
		Deck.reserve(D.Deck.size());
		for (UCHAR i=0;i<D.Deck.size();i++)
			Deck.push_back(D.Deck[i]);
		Actions.reserve(D.Actions.size());
		for (UCHAR i=0;i<D.Actions.size();i++)
		{
			Actions.push_back(D.Actions[i]);
			Actions.back().SetCardSkillProcBuffer(SkillProcs); // have to reassign buffers
		}
		Units.reserve(D.Units.size());
		for (UCHAR i=0;i<D.Units.size();i++)
		{
			Units.push_back(D.Units[i]);
			Units.back().SetCardSkillProcBuffer(SkillProcs); // have to reassign buffers
		}
		Structures.reserve(D.Structures.size());
		for (UCHAR i=0;i<D.Structures.size();i++)
		{
			Structures.push_back(D.Structures[i]);
			Structures.back().SetCardSkillProcBuffer(SkillProcs); // have to reassign buffers
		}
		bOrderMatters = D.bOrderMatters;
		bDelayFirstCard = D.bDelayFirstCard;
		memcpy(SkillProcs,D.SkillProcs,CARD_ABILITIES_MAX*sizeof(UCHAR));
		memcpy(CardPicks,D.CardPicks,DEFAULT_DECK_RESERVE_SIZE*sizeof(UINT));
		CSIndex = D.CSIndex;
		CSResult = D.CSResult;
		DamageToCommander = D.DamageToCommander;
		if (D.bOrderMatters)
		{
			Hand.clear();
			for (MSID::iterator si=D.Hand.begin();si!=D.Hand.end();si++)
				Hand.insert(*si);			
		}
		//for (VCARDS::iterator vi = D.Deck.begin();vi != D.Deck.end();vi++)
		//	Deck.push_back(*vi);
	}
	bool operator==(const ActiveDeck &D) const
	{
		if (strcmp(GetHash64().c_str(),D.GetHash64().c_str()))
			return false;
		if (Units.size() != D.Units.size())
			return false;
		for (UCHAR i=0;i<Units.size();i++)
			if (!(Units[i] == D.Units[i]))
				return false;
		if (Structures.size() != D.Structures.size())
			return false;
		for (UCHAR i=0;i<Structures.size();i++)
			if (!(Structures[i] == D.Structures[i]))
				return false;
		if (Actions.size() != D.Actions.size())
			return false;
		for (UCHAR i=0;i<Actions.size();i++)
			if (!(Actions[i] == D.Actions[i]))
				return false;
		return true;
	}
	bool operator<(const ActiveDeck &D) const
	{
		int sr = strcmp(GetHash64().c_str(),D.GetHash64().c_str());
		if (sr)
			return (sr < 0);
		if (Units.size() != D.Units.size())
			return (Units.size() < D.Units.size());
		for (UCHAR i=0;i<Units.size();i++)
			if (Units[i] != D.Units[i])
				return (Units[i] < D.Units[i]);
		if (Structures.size() != D.Structures.size())
			return (Structures.size() < D.Structures.size());
		for (UCHAR i=0;i<Structures.size();i++)
			if (Structures[i] != D.Structures[i])
				return (Structures[i] < D.Structures[i]);
		if (Actions.size() != D.Actions.size())
			return (Actions.size() < D.Actions.size());
		for (UCHAR i=0;i<Actions.size();i++)
			if (Actions[i] != D.Actions[i])
				return (Actions[i] < D.Actions[i]);
		return false;
	}
	const bool IsValid(bool bSoftCheck = false) const
	{
		if (!Commander.IsDefined())
			return false;
		if (Deck.empty())
			return true;
		if (bSoftCheck)
			return true;
		set <UINT> cards;
		bool bLegendary = false;
		for (UCHAR i=0;i<Deck.size();i++)
		{
			UINT rarity = Deck[i].GetRarity();
			if (rarity == RARITY_LEGENDARY)
			{
				if (bLegendary)
					return false;
				else
					bLegendary = true;
			}
			else
			{
				if (Deck[i].GetRarity() == RARITY_UNIQUE)
				{
					if (cards.find(Deck[i].GetId()) != cards.end())
						return false;
					else
						cards.insert(Deck[i].GetId());
				}
			}
		}
		return true;
	}
	void SetOrderMatters(const bool bMatters)
	{
		bOrderMatters = bMatters;
	}
	void DelayFirstCard()
	{
		bDelayFirstCard = true;
	}
	void Add(const Card *c)
	{
		Deck.push_back(c);
	}
	bool IsInTargets(PlayedCard *pc, PVCARDS *targets) // helper
	{
		for (PVCARDS::iterator vi=targets->begin();vi!=targets->end();vi++)
			if (pc == (*vi))
				return true;
		return false;
	}
	void ApplyEffects(PlayedCard &Src,int Position,ActiveDeck &Dest,bool IsMimiced=false,bool IsFusioned=false,PlayedCard *Mimicer=0)
	{
		UCHAR destindex,aid,faction;
		PVCARDS targets;
		targets.reserve(DEFAULT_DECK_RESERVE_SIZE);
		PlayedCard *tmp;
		UCHAR effect;
		UCHAR FusionMultiplier = 1;
		if (IsFusioned)
			FusionMultiplier = 2;

		bool bSplit = false;

		bool bIsSelfMimic = false; // Chaosed Mimic indicator
		if (IsMimiced && Mimicer && (!Units.empty()))
		{
			for (UCHAR i=0;i<Units.size();i++)
				if (Mimicer = &Units[i])
				{
					bIsSelfMimic = true;
					break;
				}
		}
		// here is a good question - can paybacked skill be paybacked? - nope
		// can paybacked skill be evaded? - doubt
		// in current model, it can't be, payback applies effect right away, without simulationg it's cast
		// another question is - can paybacked skill be evaded? it is possible, but in this simulator it can't be
		// both here and in branches
		UCHAR ac = Src.GetAbilitiesCount();
		for (UCHAR aindex=0;aindex<ac;aindex++)
		{
			if (Src.GetEffect(ACTIVATION_JAM) > 0)
				break; // card was jammed by payback (or chaos?)
			if (Src.GetEffect(ACTIVATION_FREEZE) > 0)
				break; // chaos-mimic-freeze makes this possible

			UCHAR chaos = Src.GetEffect(ACTIVATION_CHAOS); // I need to check this every time card uses skill because it could be paybacked chaos - such as Pulsifier with payback, chaos and mimic

			aid = Src.GetAbilityInOrder(aindex);
			// cleanse
			if (aid == ACTIVATION_CLEANSE)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier; // will it be fusioned? who knows
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (!(*vi)->IsCleanseTarget())
								vi = targets.erase(vi);
							else vi++;
						}
						bool bTributable = (IsMimiced && IsInTargets(Mimicer,&targets)) || ((!IsMimiced) && IsInTargets(&Src,&targets));
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (vi = targets.begin();vi != targets.end();vi++)
						{
							if (bConsoleOutput)
							{
								Src.PrintDesc();
								printf(" cleanse ");
								(*vi)->PrintDesc();
								printf("\n");
							}
							(*vi)->Cleanse();
							if (!IsMimiced)
								Src.fsSpecial += effect;
							else
								Mimicer->fsSpecial += effect;
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
							if ((*vi)->GetAbility(DEFENSIVE_TRIBUTE) && bTributable && PROC50)
							{
								if (IsMimiced)
								{
									if ((Mimicer->GetType() == TYPE_ASSAULT) && (Mimicer != (*vi)))
									{
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;
										Mimicer->Cleanse();
										(*vi)->fsSpecial += effect;
									}
								}
								else
								{
									if ((Src.GetType() == TYPE_ASSAULT) && (&Src != (*vi)))
									{
										SkillProcs[DEFENSIVE_TRIBUTE]++;
										Src.Cleanse();
										(*vi)->fsSpecial += effect;
									}
								}
							}
						}
					}
					targets.clear();
				}
			}
			// enfeeble
			if (aid == ACTIVATION_ENFEEBLE)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						if (Src.GetTargetCount(aid) != TARGETSCOUNT_ALL)
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								(*vi)->SetEffect(aid,(*vi)->GetEffect(aid) + effect);
								if (!IsMimiced)
									Src.fsSpecial += effect;
								else
									Mimicer->fsSpecial += effect;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
								if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && PROC50 && (!chaos))  // payback
								{
									Src.SetEffect(aid,Src.GetEffect(aid) + effect);
									(*vi)->fsSpecial += effect;
									Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
								}
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" enfeeble ");
									(*vi)->PrintDesc();
									printf(" for %d\n",effect);
								}
							}
					}
					targets.clear();
				}
			}
			// heal
			if (aid == ACTIVATION_HEAL)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetHealth() == (*vi)->GetMaxHealth()) || ((*vi)->IsDiseased()))
								vi = targets.erase(vi);
							else vi++;
						}
						bool bTributable = (IsMimiced && IsInTargets(Mimicer,&targets)) || ((!IsMimiced) && IsInTargets(&Src,&targets));
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (vi = targets.begin();vi != targets.end();vi++)
						{
							if (bConsoleOutput)
							{
								Src.PrintDesc();
								printf(" heals ");
								(*vi)->PrintDesc();
								printf(" for %d\n",effect);
							}
							if (!IsMimiced)
								Src.fsHealed += (*vi)->Heal(effect);
							else
								Mimicer->fsHealed += (*vi)->Heal(effect);
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
							if ((*vi)->GetAbility(DEFENSIVE_TRIBUTE) && bTributable && PROC50)
							{
								if (IsMimiced)
								{
									if ((Mimicer->GetType() == TYPE_ASSAULT) && (Mimicer != (*vi)))
									{
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;
										(*vi)->fsHealed += Mimicer->Heal(effect);
									}
								}
								else
								{
									if ((Src.GetType() == TYPE_ASSAULT) && (&Src != (*vi)))
									{
										SkillProcs[DEFENSIVE_TRIBUTE]++;
										(*vi)->fsHealed += Src.Heal(effect);
									}
								}
							}
						}
					}
					targets.clear();
				}
			}
			// supply
			if (aid == ACTIVATION_SUPPLY)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if ((effect > 0) && (Position >= 0) && ((!IsMimiced) || (Mimicer && (Mimicer->GetType() == TYPE_ASSAULT)))) // can only be mimiced by assault cards
				{
					targets.clear();
					if (Position)
						targets.push_back(&Units[Position-1]);
					targets.push_back(&Units[Position]);
					if ((DWORD)Position+1 < Units.size())
						targets.push_back(&Units[Position+1]);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetHealth() == (*vi)->GetMaxHealth()) || (*vi)->IsDiseased())
								vi = targets.erase(vi);
							else vi++;
						}
						bool bTributable = (IsMimiced && IsInTargets(Mimicer,&targets)) || ((!IsMimiced) && IsInTargets(&Src,&targets));
						for (vi = targets.begin();vi != targets.end();vi++)
						{
							if (bConsoleOutput)
							{
								Src.PrintDesc();
								printf(" supply ");
								(*vi)->PrintDesc();
								printf(" for %d\n",effect);
							}
							if (!IsMimiced)
								Src.fsHealed += (*vi)->Heal(effect);
							else
								Mimicer->fsHealed += (*vi)->Heal(effect);
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
							if ((*vi)->GetAbility(DEFENSIVE_TRIBUTE) && PROC50)
							{
								if (IsMimiced)
								{
									if ((Mimicer->GetType() == TYPE_ASSAULT) && (Mimicer != (*vi)))
									{
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;
										(*vi)->fsHealed += Mimicer->Heal(effect);
									}
								}
								else
								{
									if ((Src.GetType() == TYPE_ASSAULT) && (&Src != (*vi)))
									{
										SkillProcs[DEFENSIVE_TRIBUTE]++;
										(*vi)->fsHealed += Src.Heal(effect);
									}
								}
							}
						}
					}
					targets.clear();
				}
			}
			// protect
			// do skills like PROTECT ALL exist? in case they do - does shield override another shield?
			if (aid == ACTIVATION_PROTECT)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier; // will it be fusioned? who knows
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						/*if (Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) // if it is not PROTECT ALL then remove from targets already shielded cards
							while (vi != targets.end())
							{
								if ((*vi)->GetShield() > 0) // already shielded
									vi = targets.erase(vi);
								else vi++;
							}*/
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (vi = targets.begin();vi != targets.end();vi++)
						{
							if (bConsoleOutput)
							{
								Src.PrintDesc();
								printf(" protects ");
								(*vi)->PrintDesc();
								printf(" for %d\n",effect);
							}
							(*vi)->Protect(effect);
							if (!IsMimiced)
								Src.fsSpecial += effect;
							else
								Mimicer->fsSpecial += effect;
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
							if ((*vi)->GetAbility(DEFENSIVE_TRIBUTE) && PROC50)
							{
								if (IsMimiced)
								{
									if ((Mimicer->GetType() == TYPE_ASSAULT) && (Mimicer != (*vi)))
									{
										Mimicer->Protect(effect);
										(*vi)->fsSpecial += effect;
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;
									}
								}
								else
								{
									if ((Src.GetType() == TYPE_ASSAULT) && (&Src != (*vi)))
									{
										Src.Protect(effect);
										(*vi)->fsSpecial += effect;
										SkillProcs[DEFENSIVE_TRIBUTE]++;
									}
								}
							}
						}
					}
					targets.clear();
				}
			}
			// infuse is processed on the upper level
			// ******
			// jam
			if (aid == ACTIVATION_JAM)
			{
				effect = Src.GetAbility(aid);
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetEffect(aid)) || ((*vi)->GetWait()))
								vi = targets.erase(vi);
							else vi++;
						}
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (PROC50)
								if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
								{
									// evaded
									// no fancy stats here?!
									Dest.SkillProcs[DEFENSIVE_EVADE]++;
								}
								else
								{
									(*vi)->SetEffect(aid,effect);
									if (!IsMimiced)
										Src.fsSpecial += effect;
									else
										Mimicer->fsSpecial += effect;
									if ((!IsMimiced) || bIsSelfMimic)
										SkillProcs[aid]++;
									else
										Dest.SkillProcs[aid]++;
		/*  ?  */					if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && PROC50 && (!chaos))  // payback is it 1/2 or 1/4 chance to return jam with payback????
									{
										Src.SetEffect(aid,effect);
										(*vi)->fsSpecial += effect;
										Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
									}
									if (bConsoleOutput)
									{
										Src.PrintDesc();
										printf(" jam ");
										(*vi)->PrintDesc();
										printf("\n");
									}
								}
					}
					targets.clear();
				}
			}
			// freeze
			if (aid == ACTIVATION_FREEZE)
			{
				effect = Src.GetAbility(aid);
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetEffect(aid)))
								vi = targets.erase(vi);
							else vi++;
						}
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								// no fancy stats here?!
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								(*vi)->SetEffect(aid,effect);
								if (!IsMimiced)
									Src.fsSpecial += effect;
								else
									Mimicer->fsSpecial += effect;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
								if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && PROC50 && (!chaos)) 
								{
									Src.SetEffect(aid,effect);
									(*vi)->fsSpecial += effect;
									Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
								}
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" freeze ");
									(*vi)->PrintDesc();
									printf("\n");
								}
							}
					}
					targets.clear();
				}
			}
			// mimic - could be tricky
			if (aid == ACTIVATION_MIMIC)
			{
				effect = Src.GetAbility(aid);
				if ((effect > 0) && (!IsMimiced))
				{
					if (IsMimiced) // mimic can't be mimiced ;) it's just sad copypaste, previous line prevents this being mimiced
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" mimic ");
									(*vi)->PrintDesc();
									printf("\n");
								}
								SkillProcs[aid]++;
								if (chaos > 0)
									ApplyEffects(*(*vi),Position,*this,true,false,&Src);
								else
									ApplyEffects(*(*vi),Position,Dest,true,false,&Src);	
							}
					}
					targets.clear();
				}
			}
			// rally
			// does rally prefer units with 0 attack? ???? or its completely random
			// afaik it's completely random
			if (aid == ACTIVATION_RALLY)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetWait()) ||
								 (*vi)->GetPlayed() ||
								((*vi)->GetEffect(ACTIVATION_JAM)) || // Jammed
								((*vi)->GetEffect(ACTIVATION_FREEZE)) || // Frozen
								((*vi)->GetEffect(DMGDEPENDANT_IMMOBILIZE))   // Immobilized
								)
								vi = targets.erase(vi);
							else vi++;
						}
						bool bTributable = (IsMimiced && IsInTargets(Mimicer,&targets)) || ((!IsMimiced) && IsInTargets(&Src,&targets));
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
						{
							if (bConsoleOutput)
							{
								Src.PrintDesc();
								printf(" rally ");
								(*vi)->PrintDesc();
								printf(" for %d\n",effect);
							}
							(*vi)->Rally(effect);		
							if (!IsMimiced)
								Src.fsSpecial += effect;
							else
								Mimicer->fsSpecial += effect;
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
							if ((*vi)->GetAbility(DEFENSIVE_TRIBUTE) && bTributable && PROC50)
							{
								if (IsMimiced)
								{
									if ((Mimicer->GetType() == TYPE_ASSAULT) && (Mimicer != (*vi)))
									{
										Mimicer->Rally(effect);
										(*vi)->fsSpecial += effect;
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;
									}
								}
								else
								{
									if ((Src.GetType() == TYPE_ASSAULT) && (&Src != (*vi)))
									{
										Src.Rally(effect);
										(*vi)->fsSpecial += effect;
										Dest.SkillProcs[DEFENSIVE_TRIBUTE]++;										
									}
								}
							}
						}
					}
					targets.clear();
				}
			}
			// recharge -  only action cards
			if (aid == ACTIVATION_RECHARGE)
			{
				if (Src.GetAbility(aid) > 0)
					if (PROC50)
						Deck.push_back(Src);
			}
			// repair
			if (aid == ACTIVATION_REPAIR)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Structures,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if ((*vi)->GetHealth() == (*vi)->GetMaxHealth())
								vi = targets.erase(vi);
							else vi++;
						}
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
						{
							(*vi)->Heal(effect);
							if (!IsMimiced)
								Src.fsHealed += effect;
							else
								Mimicer->fsHealed += effect;
							if ((!IsMimiced) || bIsSelfMimic)
								SkillProcs[aid]++;
							else
								Dest.SkillProcs[aid]++;
						}
					}
					targets.clear();
				}
			}
			// shock
			if (aid == ACTIVATION_SHOCK)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					Src.fsDmgDealt += Dest.Commander.SufferDmg(effect);
					DamageToCommander += effect;
				}
			}
			// siege
			if (aid == ACTIVATION_SIEGE)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Structures,faction,targets);
					else	
						GetTargets(Dest.Structures,faction,targets);
					if (targets.size())
					{
						if (Src.GetTargetCount(aid) != TARGETSCOUNT_ALL)
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
					
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								(*vi)->fsAvoided += effect;
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								UCHAR sdmg = (*vi)->SufferDmg(effect);
								if (!IsMimiced)
									Src.fsDmgDealt += sdmg;
								else
									Mimicer->fsDmgDealt += sdmg;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
							}
					}
					targets.clear();
				}
			}
			// split
			if (aid == ACTIVATION_SPLIT)
			{
				effect = Src.GetAbility(ACTIVATION_SPLIT);
				if ((effect > 0) && (!IsMimiced))
				{
					//Units.push_back(PlayedCard(Src.GetOriginalCard()));
					// vector can be reallocated after push back, so I have to update Src
					// Src = Units[Position]; this doesn't work because I use reference to an object
					// workaround:
					bSplit = true;
				}
			}
			// strike - Only targets active Assault cards on play with at least 1 Attack that are neither Jammed nor Immobilized
			if (aid == ACTIVATION_STRIKE)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						if (Src.GetTargetCount(aid) != TARGETSCOUNT_ALL)
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								(*vi)->fsAvoided += effect;
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" strike ");
									(*vi)->PrintDesc();
									printf(" for %d\n",effect);
								}
								UCHAR sdmg = (*vi)->StrikeDmg(effect);
								if (!IsMimiced)
									Src.fsDmgDealt += sdmg;
								else
									Mimicer->fsDmgDealt += sdmg;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
								if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && PROC50 && (!chaos))  // payback
								{
									(*vi)->fsDmgDealt += Src.StrikeDmg(effect);
									Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
								}
							}			
					}
				}
			}
			// weaken
			// i've played like 20 times mission 66 just farming and noticed that AI,
			// when played beholder one of the first cards kept weakening my unit aligned
			// to beholder every fucking turn, so I died of fear almost every time enemy
			// played beholder 1-2nd card, I just couldn't kill it, Is he that smart?
			// I played rush cards so he had plenty cards to pick from to weaken
			// I fucking hate random ...
			// no, that not true, AI is dumb
			if (aid == ACTIVATION_WEAKEN)
			{
				effect = Src.GetAbility(aid) * FusionMultiplier;
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					if (chaos > 0)
						GetTargets(Units,faction,targets);
					else
						GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetWait() == 0) && 
								((*vi)->GetAttack() >= 1) && // at least 1 Attack OR it is WEAKEN ALL, that can lower ur attack down to -100500
								(!(*vi)->GetEffect(ACTIVATION_JAM)) && // neither Jammed
								(!(*vi)->GetEffect(ACTIVATION_FREEZE)) && // neither Frozen
								(!(*vi)->GetEffect(DMGDEPENDANT_IMMOBILIZE)) &&   // nor Immobilized
								((!(*vi)->GetPlayed()) || (!chaos)) // if it is chaosed - only target cards that didn't play yet
								)
								vi++; // skip
							else
								vi = targets.erase(vi);
						}
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50) && (!chaos))
							{
								// evaded
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" weaken ");
									(*vi)->PrintDesc();
									printf(" for %d\n",effect);
								}
								UCHAR we = (*vi)->Weaken(effect);
								if (!IsMimiced)
									Src.fsSpecial += we;
								else
									Mimicer->fsSpecial += we;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
								if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && (Src.GetAttack() > 0) && PROC50 && (!chaos))  // payback
								{
									(*vi)->fsSpecial += Src.Weaken(effect);
									Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
								}
							}
					}
				}
			}
			// Chaos		
			if (aid == ACTIVATION_CHAOS)
			{
				effect = Src.GetAbility(aid);
				if (effect > 0)
				{
					if (IsMimiced)
						faction = FACTION_NONE;
					else
						faction = Src.GetTargetFaction(aid);
					GetTargets(Dest.Units,faction,targets);
					if (targets.size())
					{
						PVCARDS::iterator vi = targets.begin();
						while (vi != targets.end())
						{
							if (((*vi)->GetWait() == 0) && 
								(!(*vi)->GetEffect(ACTIVATION_JAM)) && // not Jammed
								(!(*vi)->GetEffect(ACTIVATION_FREEZE)) && // not Frozen
								(!(*vi)->GetEffect(ACTIVATION_CHAOS))    // not Chaosed
								)
								vi++; // skip
							else
								vi = targets.erase(vi);
						}
						if ((Src.GetTargetCount(aid) != TARGETSCOUNT_ALL) && (!targets.empty()))
						{
							destindex = rand() % targets.size();
							tmp = targets[destindex];
							targets.clear();
							targets.push_back(tmp);
						}
						for (PVCARDS::iterator vi = targets.begin();vi != targets.end();vi++)
							if (((*vi)->GetAbility(DEFENSIVE_EVADE)) && (PROC50))
							{
								// evaded
								Dest.SkillProcs[DEFENSIVE_EVADE]++;
							}
							else
							{
								if (bConsoleOutput)
								{
									Src.PrintDesc();
									printf(" chaos ");
									(*vi)->PrintDesc();
									printf("\n");
								}
								(*vi)->SetEffect(ACTIVATION_CHAOS,effect);
								UCHAR we = effect;
								if (!IsMimiced)
									Src.fsSpecial += we;
								else
									Mimicer->fsSpecial += we;
								if ((!IsMimiced) || bIsSelfMimic)
									SkillProcs[aid]++;
								else
									Dest.SkillProcs[aid]++;
								if ((*vi)->GetAbility(DEFENSIVE_PAYBACK) && (Src.GetType() == TYPE_ASSAULT) && (Src.GetAttack() > 0) && PROC50)  // payback
								{
									Src.SetEffect(ACTIVATION_CHAOS,effect);
									(*vi)->fsSpecial += effect;
									Dest.SkillProcs[DEFENSIVE_PAYBACK]++;
								}
							}
					}
				}
			}
		}
		// split, finally, can't do it inside of the loop because it corrupts pointer to Src since vector can be moved
		if (bSplit)
		{
			Units.push_back(Src.GetOriginalCard());
			Units.back().SetCardSkillProcBuffer(SkillProcs);
		}
	}
	void SweepFancyStats(PlayedCard &pc)
	{
		if (!CSIndex) return;
		if (!CSResult) return;
		CSResult[CSIndex[pc.GetId()]].FSRecordCount++;
		CSResult[CSIndex[pc.GetId()]].FSAvoided += pc.fsAvoided;
		CSResult[CSIndex[pc.GetId()]].FSDamage += pc.fsDmgDealt;
		CSResult[CSIndex[pc.GetId()]].FSMitigated += pc.fsDmgMitigated;
		CSResult[CSIndex[pc.GetId()]].FSHealing += pc.fsHealed;
		CSResult[CSIndex[pc.GetId()]].FSSpecial += pc.fsSpecial;
	}
	void SweepFancyStatsRemaining()
	{
		if (!CSIndex) return;
		if (!CSResult) return;
		SweepFancyStats(Commander);
		for (VCARDS::iterator vi = Units.begin();vi != Units.end();vi++)
			SweepFancyStats(*vi);
		for (VCARDS::iterator vi = Structures.begin();vi != Structures.end();vi++)
			SweepFancyStats(*vi);
	}
	const Card *PickNextCard(bool bNormalPick = true)
	{
		// pick a random card
		VCARDS::iterator vi = Deck.begin();
		UCHAR indx = 0;
		if (vi != Deck.end()) // gay ass STL updates !!!
		{
			if (!bOrderMatters)
			{
				// standard random pick
				indx = rand() % Deck.size();
			}
			else
			{
				// pick that involves 'hand' and priorities
				// fill hand
				UCHAR handsize = 3;
				if (Deck.size() <= handsize)
				{
					Hand.clear();
					for (UCHAR i=0;i<Deck.size();i++)
						Hand.insert(i);
				}
				else
				{
					do
					{
						indx = rand() % Deck.size();
						// we need to pick first card of a same type, instead of picking this card
						for (UCHAR i=0;i<Deck.size();i++)
							if ((Deck[indx].GetId() == Deck[i].GetId()) && (Hand.find(indx) == Hand.end()) && (Hand.find(i) == Hand.end()))
							{
								indx = i;
								break;
							}
						if (Hand.find(indx) == Hand.end())
						{
							Hand.insert(indx);
							//printf("add to hand %d\n",indx);
						}
					}
					while (Hand.size() < handsize);
				}
				indx = (*Hand.begin()); // pick first in set
				//printf("pick %d\n",indx);
				MSID tHand;
				for (MSID::iterator si = Hand.begin(); si != Hand.end(); si++)
					if (si != Hand.begin()) // this one is picked
						tHand.insert((*si) - 1); // offset after pick
				Hand.clear();
				for (MSID::iterator si = tHand.begin(); si != tHand.end(); si++)
					Hand.insert(*si);

				/*for (MSID::iterator si = Hand.begin(); si != Hand.end(); si++)
				{
					printf("%d ",*si);
				}
				printf(" -> %d\n",indx);*/
			}
		}
		while(vi != Deck.end())
		{
			if (!indx)
			{
				const Card * c = vi->GetOriginalCard();
				if (bNormalPick)
				{
					if (bConsoleOutput)
					{
						Commander.PrintDesc();
						printf(" picks ");
						vi->PrintDesc();
						printf("\n");
					}
					if (bDelayFirstCard)
					{
						vi->IncWait();
						bDelayFirstCard = false;
					}
					if (vi->GetType() == TYPE_ASSAULT)
					{
						Units.push_back(*vi);
						Units.back().SetCardSkillProcBuffer(SkillProcs);
					}
					if (vi->GetType() == TYPE_STRUCTURE)
					{
						Structures.push_back(*vi);
						Structures.back().SetCardSkillProcBuffer(SkillProcs);
					}
					if (vi->GetType() == TYPE_ACTION)
					{
						Actions.push_back(*vi);
						Actions.back().SetCardSkillProcBuffer(SkillProcs);
					}
					for (UCHAR i=0;i<DEFAULT_DECK_RESERVE_SIZE;i++)
						if (!CardPicks[i])
						{
							CardPicks[i] = vi->GetId();
							break;
						}
					vi = Deck.erase(vi);
				}
				return c;
			}
			vi++;
			indx--;
		}
		return 0; // no cards for u
	}
	void AttackDeck(ActiveDeck &Def, bool bSkipCardPicks = false)
	{
		// process poison
		for (UCHAR i=0;i<Units.size();i++)
		{
			Units[i].ResetShield(); // according to wiki, shield doesn't affect poison, it wears off before poison procs I believe
			Units[i].ProcessPoison();
		}

		if (!bSkipCardPicks)
			PickNextCard();

		PlayedCard Empty;
		UCHAR iFusionCount = 0;
		for (UCHAR i=0;i<Structures.size();i++)
		{
			if (Structures[i].GetAbility(SPECIAL_FUSION) > 0)
				iFusionCount++;
		}
		// action cards
		for (UCHAR i=0;i<Actions.size();i++)
		{
			// apply actions somehow ...
			ApplyEffects(Actions[i],-1,Def);
		}
		for (VCARDS::iterator vi = Actions.begin();vi != Actions.end();vi++)
			SweepFancyStats(*vi);
		Actions.clear();
		// commander card
		// ok lets work out Infuse:
		// infuse - dont know how this works :(
		// ok so afaik it changes one random card from either of your or enemy deck into bloodthirsty
		// faction plus it changes heal and rally skills faction, if there were any, into bloodthirsty
		// i believe it can't be mimiced and paybacked(can assume since it's commander skill) and 
		// it can be evaded, according to forums
		// "his own units that have evade wont ever seem to evade.
		// (every time ive seen the collossus infuse and as far as i can see� he has no other non bt with evade.)"
		// so, I assume, evade works for us, but doesn't work for his cards
		// the bad thing about infuse is that we need faction as an attribute of card, we can't pull it out of
		// library, I need to add PlayedCard.Faction, instead of using Card.Faction
		// added
		if (Commander.IsDefined() && Commander.GetAbility(ACTIVATION_INFUSE) > 0)
		{
			// pick a card
			PVCARDS targets;
			targets.reserve(DEFAULT_DECK_RESERVE_SIZE);
			GetTargets(Def.Units,FACTION_BLOODTHIRSTY,targets,true);
			UCHAR defcount = targets.size();
			GetTargets(Units,FACTION_BLOODTHIRSTY,targets,true);
			if (!targets.empty())
			{
				UCHAR i = rand() % targets.size(); 
				PlayedCard *t = targets[i];
				if ((i < defcount) && (t->GetAbility(DEFENSIVE_EVADE)) && PROC50) // we check evade only on our cards, enemy cards don't seem to actually evade infuse since it's rather helpful to them then harmful
				{
					// evaded infuse
					//printf("Evaded\n");
				}
				else
					t->Infuse(FACTION_BLOODTHIRSTY);
			}
		}
		// apply actions same way 
		if (Commander.IsDefined())
			ApplyEffects(Commander,-1,Def);
		// structure cards
		for (UCHAR i=0;i<Structures.size();i++)
		{
			// apply actions somehow ...
			if (Structures[i].BeginTurn())
				ApplyEffects(Structures[i],-1,Def,false,(iFusionCount >= 3));
			Structures[i].EndTurn();
		}
		// assault cards
		for (UCHAR i=0;i<Units.size();i++)
		{
			if (Units[i].BeginTurn())
			{
				//if (!Units[i].GetEffect(ACTIVATION_JAM)) // jammed - checked in beginturn
				ApplyEffects(Units[i],i,Def);
				if ((!Units[i].GetEffect(DMGDEPENDANT_IMMOBILIZE)) && (!Units[i].GetEffect(ACTIVATION_JAM)) && (!Units[i].GetEffect(ACTIVATION_FREEZE))) // tis funny but I need to check Jam for second time in case it was just paybacked
				{
					if (Units[i].IsAlive() && Units[i].GetAttack()) // can't attack with dead unit ;) also if attack = 0 then dont attack at all
						Attack(i,Def);
				}
			}
			Units[i].EndTurn();
		}
		// refresh commander
		if (Commander.IsDefined() && Commander.GetAbility(DEFENSIVE_REFRESH)) // Bench told refresh procs at the end of player's turn
			Commander.Refresh();
		// clear dead units here yours and enemy
		if (!Units.empty())
		{
			VCARDS::iterator vi = Units.begin();
			while (vi != Units.end())
				if (!vi->IsAlive())
				{
					SweepFancyStats(*vi);
					vi = Units.erase(vi);
				}
				else
				{
					vi->ResetPlayedFlag();
					vi->ClearEnfeeble(); // this is important for chaosed skills
					if ((!vi->IsDiseased()) && (vi->GetAbility(DEFENSIVE_REFRESH))) // Bench told refresh procs at the end of player's turn
						vi->Refresh();
					vi->RemoveDebuffs(); // post-turn
					vi++;
				}
		}
		if (!Structures.empty())
		{
			VCARDS::iterator vi = Structures.begin();
			while (vi != Structures.end())
				if (!vi->IsAlive())
				{
					SweepFancyStats(*vi);
					vi = Structures.erase(vi);
				}
				else
				{
					if ((!vi->IsDiseased()) && (vi->GetAbility(DEFENSIVE_REFRESH))) // Bench told refresh procs at the end of player's turn
						vi->Refresh();
					vi++;
				}
		}
		//
		if (!Def.Units.empty())
		{
			VCARDS::iterator vi = Def.Units.begin();
			while (vi != Def.Units.end())
				if (!vi->IsAlive())
				{
					Def.SweepFancyStats(*vi);
					vi = Def.Units.erase(vi);
				}
				else
				{
					vi->ClearEnfeeble(); // this is important for chaosed skills
					vi++;
				}
		}
		if (!Def.Structures.empty())
		{
			VCARDS::iterator vi = Def.Structures.begin();
			while (vi != Def.Structures.end())
				if (!vi->IsAlive())
				{
					Def.SweepFancyStats(*vi);
					vi = Def.Structures.erase(vi);
				}
				else
					vi++;
		}
		// check if delete record from vector via iterator and then browse forward REALLY WORKS????
		// shift cards
	}
	void PrintShort()
	{
		printf("%s [",Commander.GetName());
		for (UCHAR i=0;i<Deck.size();i++)
		{
			if (i)
				printf(",");
			printf(Deck[i].GetName());
		}
		printf("]\n");
	}
	string GetDeck() const
	{
		if (Deck.empty())
			return string();
		string s;
		char buffer[10];
		if (Commander.IsDefined())
		{
			_itoa_s(Commander.GetId(),buffer,10);
			s.append(buffer);
		}
		for (UCHAR i=0;i<Deck.size();i++)
		{
			if (!s.empty())
				s.append(",");
			_itoa_s(Deck[i].GetId(),buffer,10);
			s.append(buffer);
		}
		return s;
	}
	string GetHash64(bool bCardPicks = false) const
	{
#define HASH_SAVES_ORDER	1
		if (Deck.empty() && ((!bCardPicks) || (!CardPicks[0])))
			return string();
#if HASH_SAVES_ORDER
		typedef vector<UINT> MSID; 
#else
		typedef multiset<UINT> MSID; // I <3 sets, they keep stuff sorted ;)
#endif
		MSID ids;
		if (!bCardPicks)
		{
		for (UCHAR i=0;i<Deck.size();i++)
#if HASH_SAVES_ORDER
			ids.push_back(Deck[i].GetId());
#else
			ids.insert(Deck[i].GetId());
#endif
		}
		else
			for (UCHAR i=0;(i<DEFAULT_DECK_RESERVE_SIZE) && (CardPicks[i]);i++)
				ids.push_back(CardPicks[i]); // multiset is disallowed!
		string s;
		UINT tmp = 0, t;
		unsigned short lastid = 0, cnt = 1;
		if (Commander.IsDefined())
		{
			tmp = ID2BASE64(Commander.GetId());
			s.append((char*)&tmp);
			//printf("1: %s -commander\n",(char*)&tmp);
		}
		MSID::iterator si = ids.begin();
		do
		{
			// we can actually use Id range 4000-4095 (CARD_MAX_ID - 0xFFF) for special codes,
			// adding RLE here
			tmp = ID2BASE64(*si);
			si++;
			if ((lastid != tmp) || (si == ids.end()))
			{
				if (lastid)
				{
					t = lastid;
					if (cnt == 2)
					{
						s.append((char*)&t);
						s.append((char*)&t);
						//printf("4: %s -dupe\n",(char*)&t);
						//printf("4: %s -dupe\n",(char*)&t);
						
					}
					else
						if (cnt > 2)
						{
							s.append((char*)&t);
							//printf("3: %s -value\n",(char*)&t);
							t = ID2BASE64(CARD_MAX_ID + cnt); // special code, RLE count
							s.append((char*)&t); 
							//printf("3: %s -rle\n",(char*)&t);
						}
						else
						{
							s.append((char*)&t);
							//printf("5: %s\n",(char*)&t);
						}
					cnt = 1;
				}
				if (si == ids.end())
				{
					s.append((char*)&tmp);
					//printf("2: %s\n",(char*)&tmp);
				}
				lastid = tmp;
			}
			else
				cnt++;  // RLE, count IDs
		}
		while (si != ids.end());
		return s;
	}
protected:
	void GetTargets(VCARDS &From, UCHAR TargetFaction, PVCARDS &GetTo, bool bForInfuse = false)
	{
		if (!bForInfuse)
			GetTo.clear();
		for (VCARDS::iterator vi = From.begin();vi != From.end();vi++)
			if ((vi->IsAlive()) && (((vi->GetFaction() == TargetFaction) && (!bForInfuse)) || (TargetFaction == FACTION_NONE) || ((vi->GetFaction() != TargetFaction) && (bForInfuse))))
				GetTo.push_back(&(*vi));
	}
};