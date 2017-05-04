

public class testGateway {
	private static Gateway_jni gj_inst = null; 
	public static void main(final String[] args) {

		gj_inst = new Gateway_jni();
		System.out.println("The WAN IP : " + gj_inst.getWanIp());
		gj_inst.getLoid();
		System.out.println("USB : " + gj_inst.getUSB());
	}

}

