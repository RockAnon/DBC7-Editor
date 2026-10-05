#include <string>
#include <sstream>
#include "editor.h"
#include "resource.h"
#include "aatf.h"
#ifndef UNICODE  
  typedef std::string tstring; 
  typedef std::stringstream tstringstream;
#else
  typedef std::wstring tstring; 
  typedef std::wstringstream tstringstream;
#endif

//Struct to hold the necessary data to check if each player skill matches its target rating and, if not, print an error message
struct skillCheck
{
	unsigned char c_skillRate; //The player's rating in a skill
	tstring s_skillName; //The skill name
	int n_minPesVersion; //The PES version in which skill was introduced (0 if in all versions)
	int n_targetRate; //What the player's rating in this skill should be under the rules
};

//This array encodes the mapping between the registered position IDs (0-12) used in the player_export struct's reg_pos field
// and the corresponding index in the playable position array (player_export play_pos).  It is used to check that a player has 
// playable position set to A in the same position in which they are registered.
/* position	reg_pos	play_pos
	GK		0		12
	CB		1		9
	LB		2		10
	RB		3		11
	DMF		4		5
	CMF		5		6
	LMF		6		7
	RMF		7		8
	AMF		8		4
	LWF		9		2
	RWF		10		3
	SS		11		1
	CF		12		0 */
int regPosToPlayPosMap[13] = { 12, 9, 10, 11, 5, 6, 7, 8, 4, 2, 3, 1, 0 };

//============================
//AATF Settings
int manletBonus = 5;
int silverManletBonus = 5;
int goldManletBonus = 0;
int silverGiantPen = 0;
int goldGiantPen = 0;

int goldRate = 99; //Player skill ratings
int silverRate = 88;
int regRate = 77;

int reqNumGold = 2; //Numbers of medals
int reqNumSilver = 3;

int goldForm = 8; //possible range 1-8
int silverForm = 8;
int regForm = 4;

int goldIR = 3; //Injury resistence (possible range 1-3)
int silverIR = 3;
int regIR = 1;

int goldWeakFootUse = 2; //Gold medal weak foot usage limit
int silverWeakFootUse = 2;
int regWeakFootUse = 2;

int goldWeakFootAcc = 4; //Gold medal weak foot accuracy limit
int silverWeakFootAcc = 4;
int regWeakFootAcc = 2;

int manletCardBonus = 1; //Manlets get 1 extra card
int manletWeakFootUse = 4; //Manlets get 4/4 weak foot usage/accuracy
int manletWeakFootAcc = 4;
int manletPosBonus = 1; //Manlets get 1 extra double A position

int gkSkillCards = 2; //Skill cards
int regSkillCards = 3;
int silverSkillCards = 4;
int goldSkillCards = 5;

int gkTrickCards = 0; //Trick cards
int regTrickCards = 2;
int silverTrickCards = 3;
int goldTrickCards = 3;

int regCOM = 0; //COM playing styles
int silverCOM = 1;
int goldCOM = 2;

//Green height bracket
//actually called Height bracket
int greenGiant = 6;
int greenTall = 6;
int greenMid = 5;
int greenManlet = 6;

//Red height bracket
//actually called Manlet bracket
int redGiant = 0;
int redTall = 10;
int redMid = 7;
int redManlet = 6;

//height communism bracket
//actually called Universal bracket
int commieGiant = 0;
int commieTall = 23;
int commieMid = 0;
int commieManlet = 0;

//giga bracket
//actually called GigaHeight bracket
int gigaGiant = 0;
int gigaTall = 7;
int gigaMid = 10;
int gigaManlet = 6;
int gigaExtraHeight = 50; //can be added to any players heights

int heightMax = 210; //Player heights in each category
int heightGiant = 195;
int heightTall = 185;
int heightTallGK = 189;
int heightMid = 180;
int heightManlet = 175;


void aatf_single(HWND hAatfbox, int pesVersion, int teamSel, player_entry* gplayers, team_entry* gteams, int gnum_players)
{
	player_entry player;
	tstring msgOut;
	msgOut+=_T("Team: ");
	msgOut+=gteams[teamSel].name;
	msgOut+=_T("\r\n");

	//============================

	bool hasCaptain = false;
	int numGK = 0;
	//Count of player ratings
	int numReg = 0;
	int numSilver = 0;
	int numGold = 0;
	//Count of height brackets
	int numGiant = 0;
	int numTall = 0;
	int numMid = 0;
	int numManlet = 0;
	int usedBonusHeight = 0;
	int numTallGiga = 0; //for use counting heights used if on the giga bracket
	int numMidGiga = 0;
	int numManletGiga = 0;
	bool usingRed = false;
	bool usingGreen = false;
	bool usingCommie = false;
	bool usingGiga = false;
	bool eCheck = false;

	//Run through all players once to determine height system
	for (int ii = 0; ii < gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for (int jj = 0; jj < gnum_players; jj++)
		{
			if (gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}
		if (player.height == heightManlet)
		{
			numManlet++;
		}
		else if (player.height == heightMid)
		{
			numMid++;
		}
		else if (player.height == heightTall || (player.reg_pos==0 && player.height == heightTallGK))
		{
			numTall++;
		}
		else if (player.height == heightGiant)
		{
			numGiant++;
		}
	}
	if (numManlet == greenManlet && numMid == greenMid && numTall == greenTall && numGiant == greenGiant)
	{
		usingGreen = true;
	}
	else if (numManlet == redManlet && numMid == redMid && numTall == redTall && numGiant == redGiant)
	{
		usingRed = true;
	}
	else if (numManlet == commieManlet && numMid == commieMid && numTall == commieTall && numGiant == commieGiant)
	{
		usingCommie = true;
	}
	else
	{
		usingGiga = true; //will check heights again later
	}
    
	//Now check each player for errors
    int errorTot = 0;
	for(int ii=0; ii<gteams[teamSel].num_on_team; ii++)
	{
		//Find each player on team
		for(int jj=0; jj<gnum_players; jj++)
		{
			if(gplayers[jj].id == gteams[teamSel].players[ii])
			{
				player = gplayers[jj];
				break;
			}
		}

		msgOut+=_T("\x2022 Checking ");
		msgOut+=player.name;
		//msgOut+=_T("\r\n"); //moving this to later

		tstringstream errorMsg;
		tstringstream warningMsg;

		int cardCount = 0;
        int cardMod = 0;
		int cardLimit = 0;
		int heightMod = 0;
		int weakFootUse = 0;
		int weakFootAcc = 0;
        bool hasTrick = false;
		int targetRate = 0, targetRate2 = 0, targetRate3 = 0;
		int warningRateBonus = 0; //amount a player can be above target rate with only a warning, not an error
		int rating = player.drib;
        rating = max(player.gk, rating);
        rating = max(player.finish, rating);
        rating = max(player.lowpass, rating);
        rating = max(player.loftpass, rating);
        rating = max(player.header, rating);
        rating = max(player.swerve, rating);
        rating = max(player.catching, rating);
		if (pesVersion > 15)
		{
			rating = max(player.clearing, rating);
			rating = max(player.reflex, rating);
			rating = max(player.cover, rating);
		}
        rating = max(player.body_ctrl, rating);
        if (pesVersion > 16) rating = max(player.phys_cont, rating); //Not in 16
        rating = max(player.kick_pwr, rating);
        rating = max(player.exp_pwr, rating);
        rating = max(player.ball_ctrl, rating);
        rating = max(player.ball_win, rating);
        rating = max(player.jump, rating);
        rating = max(player.place_kick, rating);
        rating = max(player.stamina, rating);
        rating = max(player.speed, rating);
		if (pesVersion > 19) rating = max(player.aggres, rating);

		/*if(player.injury+1 > 3)
		{
			errorTot++;
            errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", cannot exceed 3; ");
		}*/

		//Check if this player is the captain
		if (player.id == gteams[teamSel].players[gteams[teamSel].captain_ind]) hasCaptain = true;

		//Check if registered pos has playable set to A
		int requiredAPos = regPosToPlayPosMap[player.reg_pos];
		if(player.play_pos[requiredAPos] != 2)
		{
			errorTot++;
			errorMsg << _T("Doesn't have A in registered position; ");
		}
		
		//Count number of registered GKs
		if(player.reg_pos == 0) numGK++;

		//Count A positions
        int countA = 0;
		int countB = 0;
        for(int jj=0;jj<13;jj++)
        {
            if(player.play_pos[jj] == 2)
				countA++;
			else if (player.play_pos[jj] == 1)
				countB++;
        }
		if (countA > 1 && player.reg_pos == 0 && player.height == heightTallGK)
		{
			errorTot++;
			errorMsg << heightTallGK << _T("cm GKs may not have additional A positions; ");
		}

		//No B positions allowed:
		/*if (countB > 0)
		{
			errorTot++;
			errorMsg << _T("Has B position; ");
		}*/


        //If more than 1 A, 1 card less for each
        /*if (countA > 1)
        {
            if(player.play_pos[12] == 2) //Can't have GK as second A
			{
                errorTot++;
				errorMsg << _T("Has GK as second A position; ");
			}
            cardMod -= (countA - 1);
        }*/

		//Count cards
		int numTrick = 0;
		int numCom = 0;
		int numSkill;
		if(pesVersion==19) numSkill=39;
		else if(pesVersion>19) numSkill=41;
		else numSkill=28;
        for(int jj=0;jj<numSkill;jj++)
        {
            if(player.play_skill[jj])
            {
                cardCount++;
				//SPECIAL Winter/Spring 24: Malicia (21) is a free card
				if(jj==21) cardMod++;
                //Captain gets free captaincy card
				if(jj==25 && player.id == gteams[teamSel].players[gteams[teamSel].captain_ind])
                    cardMod++;
                //Trick cards may be free, count number
                if(jj<6 || jj==16 || jj==28 || jj==29 || jj==30 || jj==34)
				{
                    hasTrick = true;
					numTrick++;
				}
            }
        }
		for(int jj=0;jj<7;jj++)
        {
            if(player.com_style[jj])
			{
				cardCount++;
				numCom++;
			}
		}

		if(player.age<15 || player.age>50)
		{
			errorTot++;
			errorMsg << _T("Age out of range (15,50); ");
		}

		if(player.weight<max(30,player.height-129) || player.weight>(player.height-81))
		{
			errorTot++;
			errorMsg << _T("Weight out of range (") << max(30,player.height-129) << _T(",") << player.height-81 << _T("); ");
		}

		//Check playing style and registered position are in valid range per PES version
		if (player.reg_pos > 12) 
		{
			errorTot++;
			errorMsg << _T("Registered position out of range (0-12); ");
		}

		if (pesVersion <= 16)
		{
			if (player.play_style > 18 || player.play_style == 16)
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-18, excluding 16); ");
			}
		}
		else if (pesVersion > 16 && pesVersion < 19)
		{
			if (player.play_style > 17) 
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-17); ");
			}
		}
		else
		{
			if (player.play_style > 21) 
			{
				errorTot++;
				errorMsg << _T("Playing style out of range (0-21); ");
			}
		}

		/* REGULAR */
		if(rating < silverRate-silverGiantPen) //Regular player
        {
            numReg++;
			targetRate = regRate;
			targetRate2 = regRate;
			targetRate3 = regRate;
			warningRateBonus = 7; 

			weakFootUse = regWeakFootUse;
			weakFootAcc = regWeakFootAcc;

			/*if(countA > 3)
			{
				errorTot++;
				errorMsg << _T("Regular player with > 3 A positions; ");
			}*/

			/*//SPECIAL Summer 24: Malicia (21) is now mandatory on all non-medal players
			if (!player.play_skill[21])
			{
				errorTot++;
				errorMsg << _T("Malicia card is mandatory for all non-medal players; ");
			}*/

            /*if (player.form + 1 != regForm)
			{
                errorTot++;
				errorMsg << _T("Form is ") << player.form+1 << _T(", should be ") << regForm << _T("; ");
			}*/
			
            /*if (player.reg_pos == 0) //GK gets 2 cards
            {
				cardMod += min(gkTrickCards, numTrick); //1 free tricks
				cardLimit = gkSkillCards + cardMod;
                
				if(player.height > heightMid && player.height < heightTallGK)
				{
                    errorTot++;
					errorMsg << _T("GKs in this bracket must be ") << heightTallGK << _T("cm; ");
				}
				//SPECIAL Autumn 24 - GK and medals can'ts be in giant height bracket
				if (player.height >= heightGiant)
				{
					errorMsg << _T("GK heights cannot be ") << heightGiant << _T("cm; ");
				}
            }
            else
			{
				cardMod += min(regCOM, numCom); //0 free COM styles
				cardMod += min(regTrickCards, numTrick); //2 free tricks
				//cardMod += min(1, (countA - 1)); //1 free A-position
				cardLimit = regSkillCards + cardMod; //3 skill cards
			}
            
			if(player.injury+1 > regIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", should be ") << regIR << _T("; ");
			}*/

			if (player.height == heightManlet && usingRed)
			{
				targetRate += manletBonus;
				targetRate2 += manletBonus;
				targetRate3 += manletBonus;
				warningRateBonus = 5; //hits silver stats otherwise
			}

			/*//SPECIAL Spring 24: non-medals that are registered in a blue position (CB, LB, RB) get +5 defensive prowess
			if (player.reg_pos >= 1 && player.reg_pos <= 3)
			{
				targetRate2 += 5;
			}*/
			
			/*//SPECIAL Spring 24: red heights non-medals registered in a red position (CF/SS/LWF/RWF) receive a +5 boost to all stats and can stack with the boost from being 175cm however these players cannot be given a 2nd A position
			if (player.reg_pos >= 9 && player.reg_pos <= 12 && usingRed)
			{
				targetRate += 5;
				targetRate2 += 5;
				//SPECIAL Summer 24: Red Position non-medal manlet players (CF/SS/LWF/RWF) have stamina stat = 77
				if (player.height <= heightManlet && usingRed)
					targetRate3 = regRate;
				else
					targetRate3 += 5;
				if (countA > 1)
				{
					errorTot++;
					errorMsg << _T("Illegal 2nd A position on red heights non-medal forward; ");
				}
			}*/

			/*//SPECIAL Summer 24: green heights non-medals registered in a red position (CF/SS/LWF/RWF) get an additional 5cm of height except for 199cm players
			if (player.reg_pos >= 9 && player.reg_pos <= 12 && !usingRed && player.height <= heightGiant)
			{
				heightMod = 5;
			}*/

			/*if (eCheck)
			{
				if (player.reg_pos == 0)
				{
					if (numTrick < gkTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << gkTrickCards << _T("; ");
					if (numCom < regCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << regCOM << _T("; ");
				}
				else
				{
					if (numTrick < regTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << regTrickCards << _T("; ");
					if (numCom < regCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << regCOM << _T("; ");
				}
				if (player.injury + 1 < regIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << regIR << _T("; ");
			}*/
		}
		/* SILVER */
        else if(rating < goldRate-goldGiantPen) //Silver player
        {
            numSilver++;
			targetRate = silverRate;
			targetRate2 = silverRate;
			targetRate3 = silverRate;
			warningRateBonus = 5;

			weakFootUse = silverWeakFootUse;
			weakFootAcc = silverWeakFootAcc;

			msgOut += _T("      [[Silver]]");


            /*if (numSilver > reqNumSilver)
			{
                errorTot++;
				errorMsg << _T("Too many Silver medals; ");
			}
            if(player.form+1 != silverForm)
			{
                errorTot++;
				errorMsg << _T("Form is ") << player.form+1 << _T(", should be ") << silverForm << _T("; ");
			}
            if(player.reg_pos == 0) //Medals can't be GK
			{
                errorTot++;
				errorMsg << _T("Medals cannot play as GK; ");
			}
			if(player.height >= heightGiant) //HA get penalty
			{
				targetRate -= silverGiantPen;
				targetRate2 -= silverGiantPen;
				targetRate3 -= silverGiantPen;
			}*/
			if(player.height == heightManlet && usingRed)
            {
				targetRate += silverManletBonus;
				targetRate2 += silverManletBonus;
				targetRate3 += silverManletBonus;
			}
            /*cardMod += min(silverTrickCards, numTrick); //3 free tricks
			cardMod += min(silverCOM, numCom); //1 free COM
			//cardMod += min(1, (countA - 1)); //1 free A-position
			cardLimit = silverSkillCards + cardMod; //4 skill cards

			if(player.injury+1 > silverIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", should be ") << silverIR << _T("; ");
			}

			if (eCheck)
			{
				if (numTrick < silverTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << silverTrickCards << _T("; ");
				if (numCom < silverCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << silverCOM << _T("; ");
				if (player.injury + 1 < silverIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << silverIR << _T("; ");
			}*/

			//SPECIAL FAG13: Medals can trade a card for 4/4 footedness
			/*if (player.weak_use + 1 > weakFootUse)
			{
				weakFootUse = 4;
				cardLimit--;
			}*/
        }
		/* GOLD */
        else //rating == 99 //Gold player
        {
            numGold++;
			targetRate = goldRate;
			targetRate2 = goldRate;
			targetRate3 = goldRate;

			weakFootUse = goldWeakFootUse;
			weakFootAcc = goldWeakFootAcc;

			msgOut += _T("      [[Gold]]");

            /*if (numGold > reqNumGold)
			{
                errorTot++;
				errorMsg << _T("Too many Gold medals; ");
			}
            if(player.form+1 != goldForm)
			{
                errorTot++;
				errorMsg << _T("Form is ") << player.form+1 << _T(", should be ") << goldForm << _T("; ");
			}
            if(player.reg_pos == 0) //Medals can't be GK
			{
                errorTot++;
				errorMsg << _T("Medals cannot play as GK; ");
			}
			if(player.height >= heightGiant) //Medal HA penalty
			{
				targetRate -= goldGiantPen;
				targetRate2 -= goldGiantPen;
				targetRate3 -= goldGiantPen;
			}
			else if(player.height <= heightManlet && usingRed)
            {
				targetRate += goldManletBonus;
				targetRate2 += goldManletBonus;
				targetRate3 += goldManletBonus;
			}

			if (player.height > heightGiant)
			{
				errorMsg << _T("Gold heights cannot exceed ") << heightGiant << _T("cm; ");
			}*/
			
            cardMod += min(goldTrickCards, numTrick); //4 free tricks
			cardMod += min(goldCOM, numCom); //2 free COMs
			cardLimit = goldSkillCards + cardMod; //5 skill cards
           
			/*if (player.injury + 1 > goldIR)
			{
				errorTot++;
				errorMsg << _T("Injury resist is ") << player.injury+1 << _T(", should be ") << goldIR << _T("; ");
			}

			if (eCheck)
			{
				if (cardCount < 10)
				{
					if (numTrick < goldTrickCards) errorMsg << _T("WARN: Has ") << numTrick << _T(" trick cards, allowed ") << goldTrickCards << _T("; ");
					if (numCom < goldCOM) errorMsg << _T("WARN: Has ") << numCom << _T(" COM cards, allowed ") << goldCOM << _T("; ");
				}
				if (player.injury + 1 < goldIR) errorMsg << _T("WARN: Has inj resist") << player.injury + 1 << _T(", allowed ") << goldIR << _T("; ");
			}

			//SPECIAL Spring 25 - GK and gold medals can't be in giant height bracket
			if (player.height >= heightGiant)
			{
				errorMsg << _T("Gold medal heights cannot be ") << heightGiant << _T("cm; ");
			}*/

			//SPECIAL FAG13: Medals can trade a card for 4/4 footedness
			/*if (player.weak_use + 1 > weakFootUse)
			{
				weakFootUse = 4;
				cardLimit--;
			}*/
		}

		//If on Giga bracket, check player heights. If not on Giga bracket, heights are already ok.
		//count players as the tallest height possible that still has room, and count extra height used.
		//count TallGK height GKs as Tall players, or Mid players with +cm to reach Tall height, or manlet players with +cm to reach Tall height, in that order if lacking space
		if (usingGiga)
		{
			if (player.height == heightTallGK && player.reg_pos == 0) //gk has TallGK height, counts as a 185 cm player
			{
				if (numTallGiga < gigaTall)
				{
					numTallGiga++;
				}
				else if (numMidGiga < gigaMid)
				{
					numMidGiga++;
					usedBonusHeight += heightTall - heightMid;
				}
				else
				{
					numManletGiga++;
					usedBonusHeight += heightTall - heightManlet;
				}
			}
			else if (player.height > heightMax)
			{
				errorTot++;
				errorMsg << _T("Illegal height, max height is ") << heightMax << _T("cm; ");
			}
			else if (player.height >= heightTall)
			{
				if (numTallGiga < gigaTall)
				{
					numTallGiga++;
					usedBonusHeight += player.height - heightTall;
				}
				else if (numMidGiga < gigaMid)
				{
					numMidGiga++;
					usedBonusHeight += player.height - heightMid;
				}
				else
				{
					numManletGiga++;
					usedBonusHeight += player.height - heightManlet;
				}
			}
			else if (player.height >= heightMid)
			{
				if (numMidGiga < gigaMid)
				{
					numMidGiga++;
					usedBonusHeight += player.height - heightMid;
				}
				else
				{
					numManletGiga++;
					usedBonusHeight += player.height - heightManlet;
				}
			}
			else if (player.height >= heightManlet)
			{
				numManletGiga++;
				usedBonusHeight += player.height - heightManlet;
			}
			else //too short
			{
				errorTot++;
				errorMsg << _T("Illegal height") << player.height << _T(" cm, minimum is ") << heightManlet << _T("; ");
			}
		
		}


		//Check weak foot ratings
		/*if (player.weak_use + 1 > weakFootUse)
		{
			errorTot++;
			errorMsg << _T("Weak foot usage > ") << weakFootUse << _T("; ");
		}
		if (player.weak_acc + 1 > weakFootAcc)
		{
			errorTot++;
			errorMsg << _T("Weak foot accuracy > ") << weakFootAcc << _T("; ");
		}

		//Check player card count
		if (cardCount > cardLimit)
		{
			errorTot++;
			errorMsg << _T("Has ") << cardCount << _T(" cards, only allowed ") << cardLimit << _T("; ");
		}

		//Check PES skill card limit of 10
		if(cardCount-numCom > 10)
		{
            errorTot++;
			errorMsg << _T("Has ") << cardCount-numCom << _T(" skill cards, PES limit is 10, please swap to COM cards or trade for additional A positions; ");
		}

		if (eCheck)
		{
			if (cardCount < min(cardLimit,10)) errorMsg << _T("WARN: Has ") << cardCount << _T(" cards, allowed ") << cardLimit << _T("; ");
			if (player.weak_use + 1 < weakFootUse) errorMsg << _T("WARN: Has weak usage ") << player.weak_use + 1 << _T(", allowed ") << weakFootUse << _T("; ");
			if (player.weak_acc + 1 < weakFootAcc) errorMsg << _T("WARN: Has weak accuracy ") << player.weak_acc + 1 << _T(", allowed ") << weakFootAcc << _T("; ");
		}*/

		//Check individual skill ratings
		//								c_skillRate			s_skillName			n_minPesVersion	n_targetRate
		skillCheck skillChecks[25] = {	{player.drib,		_T("Dribbling"),			0,		targetRate	},
										{player.gk,			_T("Goalkeeping"),			0,		targetRate	},
										{player.finish,		_T("Finishing"),			0,		targetRate	},
										{player.lowpass,	_T("Low Pass"),				0,		targetRate	},
										{player.loftpass,	_T("Lofted Pass"),			0,		targetRate	},
										{player.header,		_T("Header"),				0,		targetRate	},
										{player.swerve,		_T("Swerve"),				0,		targetRate	},
										{player.catching,	_T("Catching"),				0,		targetRate	},
										{player.clearing,	_T("Clearing"),				16,		targetRate	},
										{player.reflex,		_T("Reflexes"),				16,		targetRate	},
										{player.body_ctrl,	_T("Body Control"),			0,		targetRate	},
										{player.phys_cont,	_T("Physical Contact"),		17,		targetRate	},
										{player.kick_pwr,	_T("Kicking Power"),		0,		targetRate	},
										{player.exp_pwr,	_T("Explosive Power"),		0,		targetRate	},
										{player.ball_ctrl,	_T("Ball Control"),			0,		targetRate	},
										{player.ball_win,	_T("Ball Winning"),			0,		targetRate	},
										{player.jump,		_T("Jump"),					0,		targetRate	},
										{player.cover,		_T("Coverage"),				16,		targetRate	},
										{player.place_kick, _T("Place Kicking"),		0,		targetRate	},
										{player.stamina,	_T("Stamina"),				0,		targetRate3	},
										{player.speed,		_T("Speed"),				0,		targetRate	},
										{player.atk,		_T("Attacking Prowess"),	0,		targetRate	},
										{player.def,		_T("Defensive Prowess"),	0,		targetRate2	},
										{player.tight_pos,	_T("Tight Possession"),		20,		targetRate	},
										{player.aggres,		_T("Aggression"),			20,		targetRate	} };

		for (int ii = 0; ii < 25; ii++)
		{

			if (skillChecks[ii].n_minPesVersion <= pesVersion && 
				     skillChecks[ii].c_skillRate > skillChecks[ii].n_targetRate + warningRateBonus)
			{
				errorTot++;
				errorMsg << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", should be <=") << skillChecks[ii].n_targetRate << _T("; ");
			}
			else if (skillChecks[ii].n_minPesVersion <= pesVersion &&
				skillChecks[ii].c_skillRate > skillChecks[ii].n_targetRate)
			{
				warningMsg << skillChecks[ii].s_skillName << _T(" is ") << skillChecks[ii].c_skillRate << _T(", default is <=") << skillChecks[ii].n_targetRate << _T("; ");
			}
		}
		msgOut += _T("\r\n");

		if(errorMsg.rdbuf()->in_avail())
		{
			errorMsg << _T("\r\n");
			msgOut+=_T("\t");
			msgOut+=errorMsg.str();
		}
		if (warningMsg.rdbuf()->in_avail())
		{
			warningMsg << _T("\r\n");
			msgOut += _T("\tWARNING: ");
			msgOut += warningMsg.str();
		}
	}
	//Team level errors
	int diff;
	tstringstream errorMsg;

	if (!hasCaptain)
	{
		errorTot++;
		errorMsg << _T("Team must have an assigned Captain; ");
	}

	//Must have at least 1 GK
	if (numGK < 1)
	{
		errorTot++;
		errorMsg << _T("Team must have a registered GK; ");
	}

	if (errorMsg.rdbuf()->in_avail())
	{
		errorMsg << _T("\r\n");
		msgOut += errorMsg.str();
		errorMsg.clear();
		errorMsg.str(tstring());
	}

    //Check heights
    if(usingGreen) //Using Green height system
    {
		msgOut+=_T("\r\nUsing Height bracket\r\n");	
    }
    else if(usingRed) //Using Red height system
    {
		msgOut+=_T("\r\nUsing Manlet bracket\r\n");
    }
	else if (usingCommie)
	{
		msgOut += _T("\r\nUsing Universal bracket\r\n");
	}
	else if (usingGiga) //Need to check if used correct number of each player type and the amount of additional height used
	{
		tstringstream gigaMsg;
		tstringstream gigaMsg2;
		gigaMsg << _T("\r\nUsing GigaHeight bracket, used ") << std::to_wstring(usedBonusHeight) << _T("cm of ") << std::to_wstring(gigaExtraHeight) << _T("cm bonus height\r\n");
		msgOut += gigaMsg.str();
		if (diff = gigaTall - numTallGiga)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			if (!(errorMsg.rdbuf()->in_avail()))
			{
				errorMsg << _T("\t");
			}
			errorMsg << _T("Has ") << numTallGiga << _T("/") << gigaTall << _T(" ") << heightTall << _T("/") << heightTallGK << _T("cm players; ");
		}
		if (diff = gigaMid - numMidGiga)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			if (!(errorMsg.rdbuf()->in_avail()))
			{
				errorMsg << _T("\t");
			}
			errorMsg << _T("Has ") << numMidGiga << _T("/") << gigaMid << _T(" ") << heightMid << _T("cm players; ");
		}
		if (diff = gigaManlet - numManletGiga)
		{
			if (diff > 0)
			{
				errorTot += diff;
			}
			else
			{
				errorTot -= diff;
			}
			if (!(errorMsg.rdbuf()->in_avail()))
			{
				errorMsg << _T("\t");
			}
			errorMsg << _T("Has ") << numManletGiga << _T("/") << gigaManlet << _T(" ") << heightManlet << _T("cm players; ");
		}
		if (usedBonusHeight > gigaExtraHeight)
		{
			errorTot++;
			if (!(errorMsg.rdbuf()->in_avail()))
			{
				errorMsg << _T("\t");
			}
			errorMsg << _T("Used ") << usedBonusHeight << _T("cm of bonus height, only allowed ") << gigaExtraHeight << _T("cm; ");
		}
		if (usedBonusHeight != gigaExtraHeight)
		{
			gigaMsg2 << _T("\tIf not meant to be using the GigaHeight bracket, have ") << numGiant << _T("x ") << heightGiant << _T("cm players, ") << numTall << _T("x ") << heightTall << _T("cm players, ") << numMid << _T("x ") << heightMid << _T("cm players, and ") << numManlet << _T("x ") << heightManlet << _T("cm players.\r\n");
			msgOut += gigaMsg2.str();
		}
	}
	if(errorMsg.rdbuf()->in_avail())
	{
		errorMsg << _T("\r\n");
		msgOut+=errorMsg.str();
		errorMsg.clear();
		errorMsg.str(tstring());
	}
    
    //Check ability stats
	int goldCount = numGold;
	int silverCount = numSilver;
	msgOut += _T("Using ");
	while (goldCount > 0)
	{
		msgOut += _T("G");
		goldCount--;
	}
	while (silverCount > 0)
	{
		msgOut += _T("S");
		silverCount--;
	}
	msgOut += _T(" medal system\r\n");
	if (numGold == 3 && numSilver == 0)
	{
		//valid
	}
	else if (numGold == 2 && numSilver == 2)
	{
		//valid
	}
	else if (numGold == 1 && numSilver == 4)
	{
		//valid
	}
	else if (numGold == 0 && numSilver == 6)
	{
		//valid
	}
	else
	{
		errorTot++;
		errorMsg << _T("\tIllegal medal system. Allowed medal systems are GGG, GGSS, GSSSS, SSSSSS; ");
	}


	if(errorMsg.rdbuf()->in_avail())
		errorMsg << _T("\r\n");
	errorMsg << _T("\r\nErrors: ") << errorTot << _T("\r\n");
	msgOut+=errorMsg.str();

	SetWindowText(GetDlgItem(hAatfbox, IDT_AATFOUT), msgOut.c_str());
	if(errorTot)
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM) _T("You ruined Christmas."));
	else
		SendDlgItemMessage(hAatfbox, IDB_AATFOK, WM_SETTEXT, 0, (LPARAM) _T("Merry Christmas!"));
}
