public class Gateway_jni {
	public native static String getWanIp();
	public native static String getLoid();
	public native static String getUSB();

	static {
		System.loadLibrary("gwinterface");
	}
}
