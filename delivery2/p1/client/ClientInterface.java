package client;

import java.rmi.Remote;
import java.rmi.RemoteException;

/*
Remote interface of the Client that must make the setXXXXXXX() methods public so that the www server can invoke them via RMI.
*/
public interface ClientInterface extends Remote {
    // TODO
    public void setExistFile(Boolean answer) throws java.rmi.RemoteException;

    public void setGetSize(Long val) throws java.rmi.RemoteException;

    public void setGetChunk(byte[] chunk) throws java.rmi.RemoteException;
}
