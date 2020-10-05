/*
 * CanHack.h
 *
 *  Created on: 21 вер. 2020 р.
 *      Author: Tesla
 */

#ifndef SRC_CANHACK_H_
#define SRC_CANHACK_H_
#include "main.h"
#include "print.h"
#include "UserStl.h"

class CanHack
{

public:
	CanHack()
	{
		id_count = 0;
	}

	void beging(CAN_HandleTypeDef *_phcan)
	{
		phcan = _phcan;

		TxMessage.StdId 	  = 0x0;
		TxMessage.ExtId 	  = 0x000;
		TxMessage.IDE 	  = CAN_ID_STD;
		TxMessage.RTR 	  = CAN_RTR_DATA;
		TxMessage.DLC 	  = 0x8;

		for(auto &d:TxData) d = 0;

		TxMailbox         = CAN_TX_MAILBOX0;

		inits(TxMessage.StdId);
	}

	CAN_FilterTypeDef     sFilterConfig;
	CAN_TxHeaderTypeDef   TxMessage;
	CAN_RxHeaderTypeDef   RxMessage;
	uint8_t               TxData[8];
	uint8_t               RxData[8];
	uint32_t              TxMailbox;
	CAN_HandleTypeDef    *phcan;
	int size;

	uint16_t IDs[2048];
	uint16_t id_count ;
	bool CAN_Lock, CAN_FilterLock;

	bool inits(uint16_t id)
	{

/*
		CAN_FilterTypeDef  sFilterConfig;
		  uint16_t StdIdArray1[10] ={0x7D1,0x7D2,0x7D3,0x7D4,0x7D5,0x7D6,0x7D7,0x7D8,0x7D9,0x7DA}; // Визначаємо першу групу стандартного ідентифікатора CAN
		  uint16_t StdIdArray2[10] ={0x751,0x752,0x753,0x754,0x755,0x756,0x757,0x758,0x759,0x75A}; // Визначимо другу групу стандартного ідентифікатора CAN
		  uint16_t      mask,tmp,i,num;

		  sFilterConfig.FilterBank = 1;					// Використовуємо фільтр 1
		  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;			//配置为掩码模式
		  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;		//设为16位宽

		  // Налаштування першої пари фільтра
		  sFilterConfig.FilterIdLow =StdIdArray1[0]<<5;			// Встановіть перший код перевірки
		  mask =0x7ff;
		  num =sizeof(StdIdArray1)/sizeof(StdIdArray1[0]);
		  for(i =0; i<num; i++)							// Обчислимо перший код маски
		  {
		    tmp =StdIdArray1[i] ^ (~StdIdArray1[0]);
		    mask &=tmp;
		  }
		  sFilterConfig.FilterMaskIdLow =(mask<<5)|0x10;    // Отримувати лише кадри даних

		  // Налаштування другої пари фільтрів
		  sFilterConfig.FilterIdHigh = StdIdArray2[0]<<5;	// Встановіть другий код перевірки
		  mask =0x7ff;
		  num =sizeof(StdIdArray2)/sizeof(StdIdArray2[0]);
		  for(i =0; i<num; i++)					// Обчислити другий код маски
		  {
		    tmp =StdIdArray2[i] ^ (~StdIdArray2[0]);
		    mask &=tmp;
		  }
		  sFilterConfig.FilterMaskIdHigh = (mask<<5)|0x10;  // Отримувати лише кадри даних


		  sFilterConfig.FilterFIFOAssignment = 0;		// повідомлення CAN поміщається в FIFO0
		  sFilterConfig.FilterActivation = ENABLE;
		  sFilterConfig.SlaveStartFilterBank = 14;

		if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
		  {
		    Error_Handler();
		  }
*/


		for(auto &ids:IDs) ids = 0;

		sFilterConfig.FilterBank= 0;
		sFilterConfig.FilterMode= CAN_FILTERMODE_IDMASK;
		sFilterConfig.FilterScale= CAN_FILTERSCALE_32BIT;
		sFilterConfig.FilterIdHigh= 0x0000;
		sFilterConfig.FilterIdLow= 0x0000;
		sFilterConfig.FilterMaskIdHigh= 0x0000;
		sFilterConfig.FilterMaskIdLow= 0x0000;
		sFilterConfig.FilterFIFOAssignment= CAN_RX_FIFO0;
		sFilterConfig.FilterActivation= ENABLE;
		sFilterConfig.SlaveStartFilterBank = 14;

		HAL_CAN_ConfigFilter(phcan, &sFilterConfig);

		TxMessage.StdId= id;
		TxMessage.RTR= CAN_RTR_DATA;
		TxMessage.IDE= CAN_ID_STD;
		TxMessage.DLC= 8;
		TxMessage.TransmitGlobalTime = DISABLE;


	    HAL_CAN_ActivateNotification(phcan, CAN_IT_RX_FIFO0_MSG_PENDING); //enable interrupts

	    if(HAL_CAN_Start(phcan) != HAL_OK)
	    {
	    	return false;
	    }

	    return true;
	}

	void send( uint16_t _id, uint8_t *data )
	{
		static int count = 0;

		TxMessage.StdId = _id;
		TxMessage.ExtId = 0x00;
		TxMessage.RTR = 0;
		TxMessage.IDE = CAN_ID_STD;
		TxMessage.DLC = 8;

		memcpy(TxData, data, 8);

		HAL_StatusTypeDef ST = HAL_CAN_AddTxMessage(phcan, &TxMessage, data, &TxMailbox);
		if( ST == HAL_OK )
		{
			 Printf("Send[%d]-OK\n", count++);
		}
		else
		{
			Printf("Send[%d]-ERROR\n", count++);
		}
	}

	bool is_can_msg_pending()
	{
		return ((phcan->Instance->RF0R & CAN_RF0R_FMP0) > 0); //if there is no data
	}

	void Proccess()
	{
		if(is_can_msg_pending())
		{
			CAN_RxFifo0MsgPendingCallback();
		}

		if( UserInterface.len )// receive form USB
		{
			if(UserInterface.FromUSB_Data[0] == '#')
			{
				using namespace ustl;
				int pk[32] = {0};
				atoiArr((char*)UserInterface.FromUSB_Data, pk, (char*)" ");
				uint8_t spk[11] = {0}; memcpy(spk, pk, 11);
				send(spk[0], &spk[1]);
			}
			else
				if(UserInterface.FromUSB_Data[0] == '@')
				{
					if(strstr((char*)UserInterface.FromUSB_Data, "CLRF"))
					{
						CAN_Lock = true;
						 for(auto &id:IDs) { id = 0; }
						 id_count = 0;

						 CAN_Lock = false;
						Printf("CLRF OK\n");
					}
					else if(strstr((char*)UserInterface.FromUSB_Data, "DevCanHack"))
					{
						Printf("DevCanHack OK\n");
					}
					else if(strstr((char*)UserInterface.FromUSB_Data, "LFTR1"))
					{
						CAN_FilterLock = true;
						Printf("LFTR1 OK\n");
					}else if(strstr((char*)UserInterface.FromUSB_Data, "LFTR0"))
					{
						CAN_FilterLock = false;
						Printf("LFTR0 OK\n");
					}else if(strstr((char*)UserInterface.FromUSB_Data, "SilentOn"))
					{
						HAL_CAN_Stop(phcan);
						phcan->Instance->BTR |= CAN_BTR_SILM;
						HAL_CAN_Start(phcan);
						Printf("SilentOn OK\r\n");
					}
					else if(strstr((char*)UserInterface.FromUSB_Data, "SilentOff"))
					{
						HAL_CAN_Stop(phcan);
						phcan->Instance->BTR &= ~CAN_BTR_SILM;
						HAL_CAN_Start(phcan);
						Printf("SilentOff OK\r\n");
					}
				}
			UserInterface.len = 0;
		}
	}

	 void can_Init(uint8_t mode)
	 {
	   HAL_CAN_DeInit(phcan);

	   phcan->Instance = CAN1;
	   phcan->Init.Prescaler = 9;
	   if(mode)
	   phcan->Init.Mode = CAN_MODE_NORMAL;
	   else
	   phcan->Init.Mode = CAN_MODE_SILENT;
	   phcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
	   phcan->Init.TimeSeg1 = CAN_BS1_4TQ;
	   phcan->Init.TimeSeg2 = CAN_BS2_3TQ;
	   phcan->Init.TimeTriggeredMode    = DISABLE;
	   phcan->Init.AutoBusOff 		    = DISABLE;
	   phcan->Init.AutoWakeUp 		    = DISABLE;
	   phcan->Init.AutoRetransmission   = DISABLE;
	   phcan->Init.ReceiveFifoLocked    = ENABLE;
	   phcan->Init.TransmitFifoPriority = DISABLE;
	   if (HAL_CAN_Init(phcan) != HAL_OK)
	   {
	     Error_Handler();
	   }
	 }

	 uint32_t FMP0_St,FMP0_St2;
	void CAN_RxFifo0MsgPendingCallback()
	{
		 /* get can receive data */
		while ((phcan->Instance->RF0R & CAN_RF0R_FMP0) != 0){	// message pending FIFO0
		HAL_CAN_GetRxMessage(phcan, CAN_RX_FIFO0, &RxMessage, RxData);
		}
		FMP0_St = phcan->Instance->RF0R;
		if(phcan->Instance->RF0R & 3)
		{
			FMP0_St2 = phcan->Instance->RF0R;
			phcan->Instance->RF0R |= CAN_RF0R_RFOM0;
		}


		 if(!CAN_Lock)
		 {
			 uint16_t StdId = RxMessage.StdId;

			 int cnt = 0;
			 for(auto id:IDs)
			 {
				 if(id ==  RxMessage.StdId)
				 {
					 StdId = 0;
					 break;
				 }
				 else
				 if(cnt >= id_count)
				 {
				   break;
				 }

				 cnt++;
			 }

			 if(StdId)
			 {
				 if(!CAN_FilterLock)
					 IDs[id_count++] = StdId;

				 char senSer[64] = {0}; int shift = 0;

				 shift += sprintf(&senSer[shift], "%03X ", StdId);

				 for(auto &b:RxData)
				 shift += sprintf(&senSer[shift], "%02X ", b);

				 sprintf(&senSer[shift], "\n");

				 Printf(senSer);
			 }
		 }

	}

	/**
	  * @brief  Cancels a transmit request.
	  * @param CANx: where x can be 1 to select the CAN peripheral.
	  * @param mbx: Mailbox number.
	  * @retval : None.
	  */
	void can_cancel(uint8_t mbx)
	{
		/* abort transmission */
		switch (mbx)
		{
		case 0:
			phcan->Instance->TSR |= CAN_TSR_ABRQ0;
			break;
		case 1:
			phcan->Instance->TSR |= CAN_TSR_ABRQ1;
			break;
		case 2:
			phcan->Instance->TSR |= CAN_TSR_ABRQ2;
			break;
		default:
			break;
		}
	}

};

#endif /* SRC_CANHACK_H_ */
