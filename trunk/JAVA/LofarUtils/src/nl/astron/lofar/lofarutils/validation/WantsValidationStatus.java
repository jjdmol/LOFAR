package nl.astron.lofar.lofarutils.validation;

public interface WantsValidationStatus {
    void validateFailed();  // Called when a component has failed validation.
    void validatePassed();  // Called when a component has passed validation.
}

