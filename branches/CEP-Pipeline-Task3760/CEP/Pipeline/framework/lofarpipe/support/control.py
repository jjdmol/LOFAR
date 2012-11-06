#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                        Pipeline control recipe
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from lofarpipe.support.stateful import StatefulRecipe
from lofarpipe.support.lofarexceptions import PipelineException

#                                             Standalone Pipeline Control System
# ------------------------------------------------------------------------------

class control(StatefulRecipe):
    """
    Basic pipeline control framework.

    Define a pipeline by subclassing and provding a body for the
    :meth:`pipeline_logic`.

    This class provides little, but can be specialised to eg provide a
    MAC/SAS interface etc.
    """
    inputs = {}
    def pipeline_logic(self):
        """
        Define pipeline logic here in subclasses
        """
        raise NotImplementedError

    def go(self):
        super(control, self).go()

        self.logger.info(
            "LOFAR Pipeline (%s) starting." %
            (self.name,)
        )

        try:
            self.pipeline_logic()
        except Exception, message:
            self.logger.error("*******************************************")
            self.logger.error(message)
            self.logger.error("We are in control after pipeline_logic exception WINNNNNNn")
            mail_list = ["klijn@astron.nl", "nonoice@gmail.com"]
            msg = "Just some random unstructure data!"

            for entry in mail_list:
                self._mail_msg_to("lce072@astron.nl", entry,
                         "Fail pipeline run", msg)

            return 1

        return 0

    def _mail_msg_to(self, adr_from, adr_to, subject, msg):
        # Import smtplib for the actual sending function
        import smtplib

        # Import the email modules we'll need
        from email.mime.text import MIMEText

        # Create a text/plain message
        msg = MIMEText(msg)

        # me == the sender's email address
        # you == the recipient's email address
        msg['Subject'] = subject
        msg['From'] = adr_from
        msg['To'] = adr_to

        # Send the message via our own SMTP server, but don't include the
        # envelope header.
        s = smtplib.SMTP('smtp.lofar.eu')
        s.sendmail(adr_from, [adr_to], msg.as_string())
        s.quit()

