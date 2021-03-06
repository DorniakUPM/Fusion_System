////////////////////////////////
// RTMaps SDK Component
////////////////////////////////

////////////////////////////////
// Purpose of this module :
////////////////////////////////

#include "maps_Grafics.h"	// Includes the header of this component

const MAPSTypeFilterBase ValeoStructure = MAPS_FILTER_USER_STRUCTURE(AUTO_Objects);
// Use the macros to declare the inputs
MAPS_BEGIN_INPUTS_DEFINITION(MAPSGrafics)
	MAPS_INPUT("LaserObject", ValeoStructure, MAPS::FifoReader)
	MAPS_INPUT("CameraObject", ValeoStructure, MAPS::FifoReader)
    //MAPS_INPUT("iName",MAPS::FilterInteger32,MAPS::FifoReader)
MAPS_END_INPUTS_DEFINITION

// Use the macros to declare the outputs
MAPS_BEGIN_OUTPUTS_DEFINITION(MAPSGrafics)
    //MAPS_OUTPUT("oName",MAPS::Integer32,NULL,NULL,1)
MAPS_END_OUTPUTS_DEFINITION

// Use the macros to declare the properties
MAPS_BEGIN_PROPERTIES_DEFINITION(MAPSGrafics)
    //MAPS_PROPERTY("pName",128,false,false)
MAPS_END_PROPERTIES_DEFINITION

// Use the macros to declare the actions
MAPS_BEGIN_ACTIONS_DEFINITION(MAPSGrafics)
    //MAPS_ACTION("aName",MAPSGrafics::ActionName)
MAPS_END_ACTIONS_DEFINITION

// Use the macros to declare this component (Grafics) behaviour
MAPS_COMPONENT_DEFINITION(MAPSGrafics,"Grafics","1.0",128,
			  MAPS::Threaded,MAPS::Threaded,
			  -1, // Nb of inputs. Leave -1 to use the number of declared input definitions
			  -1, // Nb of outputs. Leave -1 to use the number of declared output definitions
			  -1, // Nb of properties. Leave -1 to use the number of declared property definitions
			  -1) // Nb of actions. Leave -1 to use the number of declared action definitions

//Initialization: Birth() will be called once at diagram execution startup.			  
void MAPSGrafics::Birth()
{
	direction = dir;
	firstL = true;
	firstC = true;


	direction = direction + std::to_string((int)idLaser) + "_" + std::to_string((int)idCamera) + "\\";
	_mkdir(direction.c_str());

	fLaser.open(direction + "Laser_" + std::to_string((int)idLaser) + "_" + std::to_string((int)idCamera) + ".m", ofstream::out);
	fCamera.open(direction + "Camera_" + std::to_string((int)idLaser) + "_" + std::to_string((int)idCamera) + ".m", ofstream::out);

	fLaser << "%timestamp,x_rel,y_rel,x_sigma,y_sigma,speed,speed_x_rel,speed_y_rel,speed_x_sigma,speed_y_sigma" << '\n' << "LaserN=[";
	fCamera << "%timestamp,x_rel,y_rel,x_sigma,y_sigma,speed,speed_x_rel,speed_y_rel,speed_x_sigma,speed_y_sigma" << '\n' << "Camara=[";

}

//ATTENTION: 
//	Make sure there is ONE and ONLY ONE blocking function inside this Core method.
//	Consider that Core() will be called inside an infinite loop while the diagram is executing.
//	Something similar to: 
//		while (componentIsRunning) {Core();}
//
//	Usually, the one and only blocking function is one of the following:
//		* StartReading(MAPSInput& input); //Data request on a single BLOCKING input. A "blocking input" is an input declared as FifoReader, LastOrNextReader, Wait4NextReader or NeverskippingReader (declaration happens in MAPS_INPUT: see the beginning of this file). A SamplingReader input is non-blocking: StartReading will not block with a SamplingReader input.
//		* StartReading(int nCount, MAPSInput* inputs[], int* inputThatAnswered, int nCountEvents = 0, MAPSEvent* events[] = NULL); //Data request on several BLOCKING inputs.
//		* SynchroStartReading(int nb, MAPSInput** inputs, MAPSIOElt** IOElts, MAPSInt64 synchroTolerance = 0, MAPSEvent* abortEvent = NULL); // Synchronized reading - waiting for samples with same or nearly same timestamps on several BLOCKING inputs.
//		* Wait(MAPSTimestamp t); or Rest(MAPSDelay d); or MAPS::Sleep(MAPSDelay d); //Pauses the current thread for some time. Can be used for instance in conjunction with StartReading on a SamplingReader input (in which case StartReading is not blocking).
//		* Any blocking grabbing function or other data reception function from another API (device driver,etc.). In such case, make sure this function cannot block forever otherwise it could freeze RTMaps when shutting down diagram.
//**************************************************************************/
//	In case of no blocking function inside the Core, your component will consume 100% of a CPU.
//  Remember that the StartReading function used with an input declared as a SamplingReader is not blocking.
//	In case of two or more blocking functions inside the Core, this is likely to induce synchronization issues and data loss. (Ex: don't call two successive StartReading on FifoReader inputs.)
/***************************************************************************/
void MAPSGrafics::Core() 
{
	readInputs();
}

//De-initialization: Death() will be called once at diagram execution shutdown.
void MAPSGrafics::Death()
{
	fLaser << "];";
	fCamera << "];";
	fLaser.close();
	fCamera.close();
}

void MAPSGrafics::readInputs()
{
	MAPSInput* inputs[2] = { &Input("LaserObject"), &Input("CameraObject")};
	int inputThatAnswered;
	MAPSIOElt* ioeltin = StartReading(2, inputs, &inputThatAnswered);
	if (ioeltin == NULL) {
		return;
	}
	switch (inputThatAnswered)
	{
	case 0:
		ArrayLaserObjects_input = static_cast<AUTO_Objects*>(ioeltin->Data());
		ArrayLaserObjects = *ArrayLaserObjects_input;
		WriteLaser();
	break;
	case 1:
		ArrayCameraObjects_input = static_cast<AUTO_Objects*>(ioeltin->Data());
		ArrayCameraObjects = *ArrayCameraObjects_input;
		WriteCamera();
		break;
	default:
		break;
	}
}

void MAPSGrafics::WriteLaser()
{
	for (int i = 0; i < ArrayLaserObjects.number_of_objects; i++)
	{
		if (ArrayLaserObjects.object[i].id == idLaser)
		{
			if (!firstL)
			{
				fLaser << ";" << '\n' << ArrayLaserObjects.timestamp << "," << ArrayLaserObjects.object[i].x_rel << "," << ArrayLaserObjects.object[i].y_rel << "," << ArrayLaserObjects.object[i].x_sigma
					<< "," << ArrayLaserObjects.object[i].y_sigma << "," << ArrayLaserObjects.object[i].speed_rel << "," << ArrayLaserObjects.object[i].speed_x_rel << "," << ArrayLaserObjects.object[i].speed_y_rel
					<< "," << ArrayLaserObjects.object[i].speed_x_sigma << "," << ArrayLaserObjects.object[i].speed_y_sigma;
			}
			else
			{
				fLaser << '\n' << ArrayLaserObjects.timestamp << "," << ArrayLaserObjects.object[i].x_rel << "," << ArrayLaserObjects.object[i].y_rel << "," << ArrayLaserObjects.object[i].x_sigma
					<< "," << ArrayLaserObjects.object[i].y_sigma << "," << ArrayLaserObjects.object[i].speed_rel << "," << ArrayLaserObjects.object[i].speed_x_rel << "," << ArrayLaserObjects.object[i].speed_y_rel
					<< "," << ArrayLaserObjects.object[i].speed_x_sigma << "," << ArrayLaserObjects.object[i].speed_y_sigma;
				firstL = false;
			}
		}
	}
}

void MAPSGrafics::WriteCamera()
{
	for (int i = 0; i < ArrayCameraObjects.number_of_objects; i++)
	{
		if (ArrayCameraObjects.object[i].id == idCamera)
		{
			if (!firstC)
			{
				fCamera << ";" << '\n' << ArrayCameraObjects.timestamp << "," << ArrayCameraObjects.object[i].x_rel << "," << ArrayCameraObjects.object[i].y_rel << "," << ArrayCameraObjects.object[i].x_sigma
					<< "," << ArrayCameraObjects.object[i].y_sigma << "," << ArrayCameraObjects.object[i].speed_rel << "," << ArrayCameraObjects.object[i].speed_x_rel << "," << ArrayCameraObjects.object[i].speed_y_rel
					<< "," << ArrayCameraObjects.object[i].speed_x_sigma << "," << ArrayCameraObjects.object[i].speed_y_sigma;
			}
			else
			{
				fCamera << '\n' << ArrayCameraObjects.timestamp << "," << ArrayCameraObjects.object[i].x_rel << "," << ArrayCameraObjects.object[i].y_rel << "," << ArrayCameraObjects.object[i].x_sigma
					<< "," << ArrayCameraObjects.object[i].y_sigma << "," << ArrayCameraObjects.object[i].speed_rel << "," << ArrayCameraObjects.object[i].speed_x_rel << "," << ArrayCameraObjects.object[i].speed_y_rel
					<< "," << ArrayCameraObjects.object[i].speed_x_sigma << "," << ArrayCameraObjects.object[i].speed_y_sigma;
				firstC = false;
			}
		}
	}
}
