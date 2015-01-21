
#include <windows.h>
#include <stdint.h>

#include "carcassone.h"
#include "carcassone_intrinsics.h"

internal void
DrawRectangle(game_screen_information *GameScreenInformation, vector2 CenterPosition, uint32 SideLength,
              color Color)
{
	uint32 ColorBuilt = ((RoundReal32ToUInt32(Color.R * 255.0f) << 16) |
	                     (RoundReal32ToUInt32(Color.G * 255.0f) << 8) |
	                     (RoundReal32ToUInt32(Color.B * 255.0f) << 0));

	uint8 *Row = ((uint8 *)GameScreenInformation->PixelMemory + 
	              (CenterPosition.X * GameScreenInformation->BytesPerPixel) + 
	              (CenterPosition.Y * GameScreenInformation->Pitch));

	for (uint32 Y = CenterPosition.Y - SideLength;
	     Y < (CenterPosition.Y + SideLength);
	     ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (uint32 X = CenterPosition.X - SideLength;
		     X < CenterPosition.X + SideLength;
		     ++X)
		{
			*Pixel++ = ColorBuilt;
		}

		Row += GameScreenInformation->Pitch;
	}
}

extern "C" GAME_UPDATE_AND_RENDER (GameUpdateAndRender)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if (!GameMemory->IsInitialized)
	{		
		GameState->SquarePosition.X = 200;
		GameState->SquarePosition.Y = 200;
		GameState->SquareSideLength = 10;

		GameMemory->IsInitialized = true;
	}

	if (GameInput.DPadUp)
	{
		--GameState->SquarePosition.Y;
	}
	if (GameInput.DPadDown)
	{
		++GameState->SquarePosition.Y;
	}
	if (GameInput.DPadLeft)
	{
		--GameState->SquarePosition.X;
	}
	if (GameInput.DPadRight)
	{
		++GameState->SquarePosition.X;
	}

	color SquareColor = color {0, 0, 0, 0};
	DrawRectangle(GameScreenInformation, GameState->SquarePosition, GameState->SquareSideLength, SquareColor);
}