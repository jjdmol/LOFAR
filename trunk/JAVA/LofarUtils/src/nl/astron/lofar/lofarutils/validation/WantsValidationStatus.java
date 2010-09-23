package nl.astron.lofar.lofarutils.validation;

/*
 * Include this interface if you want that a parentpanel/frame/dialog gets a call if validation failed/passed
 */
public interface WantsValidationStatus {
    void validateFailed();  // Called when a component has failed validation.
    void validatePassed();  // Called when a component has passed validation.
}

