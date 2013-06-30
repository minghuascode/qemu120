/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package s4jclient;

import com.G2.SNMP.Server.TestSNMPAgent;
import java.io.IOException;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;

/**
 *
 * @author me
 */
public class S4jclient {

    private static final Logger logger = Logger.getLogger(S4jclient.class);
   
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws IOException {
        // TODO code application logic here
        BasicConfigurator.configure();
        logger.debug("Hello world.");
        logger.info("What a beatiful day.");
        
        TestSNMPAgent.main(args);
    }
}

/*
 * http://www.mail-archive.com/snmp4j@agentpp.org/msg02023.html
 * 
Re: [SNMP4J] How to enable logging?
Frank Fock Wed, 25 Apr 2012 23:18:58 -0700
 * 
To adapt to Log4J you need to place the following code
somewhere in your main():

   // initialize Log4J logging
   static {
     LogFactory.setLogFactory(new Log4jLogFactory());
   }
 * 
> I need to enable the logging features in SNMP4j. How can I achieve that?
> For our projects we use log4j, so I have log4j.xml at the top of
> application's classpath configured the way it logs to console.
>
> It's working pretty fine.
>
> Now I want to enable logging for SNMP4j so I just put the following lines:
>
> <category name="org.snmp4j">
> <priority value="DEBUG"/>
> </category>
* 
*/
