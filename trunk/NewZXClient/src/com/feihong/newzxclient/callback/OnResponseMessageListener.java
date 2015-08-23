package com.feihong.newzxclient.callback;

import zaoxun.Msg;

/**
 * @author hao.xiong
 * @version 1.0.0
 */
public interface OnResponseMessageListener {
    /**
     *
     * @param msg
     */
	
	//public void responseLoin(Msg.LoginResponse msg);
	
    public void responseOilQuantity(Msg.OilQuantityResponse msg);
    public void responseProduction(Msg.ProductionResponse msg);
    public void responseSpeedLimit(Msg.SpeedLimitMessage msg);
    public void responseGPS(Msg.GPSData msg);
    public void responseAudioFile(Msg.AudioFileData msg);
    public void responseShortMessage(Msg.ShortMessage msg);
    
    public void responseDirection(double direction);
    
    public void responseLoadRequest(Msg.LoadRequest msg);
    public void responseLoadResponse(Msg.LoadResponse msg);
    public void responseGetEdippersResponse(Msg.GetEDippersResponse msg);
    public void responseLoadCompleteRequest(Msg.LoadCompleteRequest msg);
    public void responseLoadCompleteResponse(Msg.LoadCompleteResponse msg);
    public void responseUnloadResponse(Msg.UnloadResponse msg);
    public void responseMaterialsNotification(Msg.MaterialsNotification msg);
    
    public void responseCancelLoadRequest(Msg.CancelLoadRequest msg);
    public void responseCancelLoadResponse(Msg.CancelLoadResponse msg);
    public void responseRejectCancelRequest(Msg.RejectLoadRequest msg);
    public void responseRejectCancelResponse(Msg.RejectLoadResponse msg);
    
    public void responseDeviceStatusRequest(Msg.DeviceStatusRequest msg);
    public void responseAntiCollisionNotification(Msg.AntiCollisionNotification msg);
    
    public void responseMaterialsResponse(Msg.MaterialsResponse msg);
    public void responseCommandRequest(Msg.CommandRequest msg);
    public void responseProductionInfoResponse(Msg.ProductionInfoResponse msg);
    
    public void responseFuelRequest(Msg.FuelRequest msg);
    public void responseFuelResponse(Msg.FuelResponse msg);
    
    public void responseTargetNotification(Msg.TargetNotification msg);
    
    public void responseManualLoadCompleteRequest(Msg.ManualLoadCompleteRequest msg);
    //public void responseManualLoadCompleteResponse(Msg.ManualLoadCompleteResponse msg);
    public void responseManualUnloadRequest(Msg.ManualUnloadRequest msg);
    //public void responseManualUnloadResponse(Msg.ManualUnloadResponse msg);
}
