<?php

/*
 * Controller layer of the BotchaForm objects.
 */
/**
 * Special class to abstract operations with form_id. Casual usage is getting
 * all recipe books by form_id. It could be done by adding one more method to
 * Botcha (such as "getRecipebookByFormId") but in long-time perspective
 * it is counter-intuitive to extend our model by this way.
 * The way it is done here is as follows:
 * Botcha::getForm($form_id)
 *   ->getRecipebook();
 * It looks like additional layer of abstraction after DatabaseAbstractionLayer.
 * It helps us to get necessary data - while we don't have to repeat ourselves
 * in many places, writing queries to the database. All logic is hidden under
 * this simple interface: getForm, getRecipebook, getRecipe.
 */
class BotchaForm {

  protected $recipebook;

  protected function __construct($form_id) {
    $this->id = $form_id;
  }

  /**
   * @todo BotchaForm getForm Description.
   * @param string $form_id
   * @param boolean $create
   *   Determines should we construct new form or return NULL if it does not exist.
   * @return BotchaForm
   */
  public static function getForm($form_id, $create = TRUE) {
    $botcha_form = NULL;
    $form = BotchaFormModel::getForm($form_id);
    if (!$form && !$create) {
      $botcha_form = new BotchaFormNone($form_id);
    }
    else { //($form || $create)
      $botcha_form = new BotchaForm($form_id);
    }
    return $botcha_form;
  }

  public function setRecipebook($rbid) {
    $this->recipebook = $rbid;
    return $this;
  }

  function unsetRecipebook() {
    unset($this->recipebook);
    return $this;
  }

  /**
   * @todo BotchaForm getRecipebook Description.
   * @return BotchaRecipebook
   */
  public function getRecipebook() {
    return Botcha::getRecipebook(!empty($this->recipebook) ? $this->recipebook : 'none', FALSE);
  }

  public function save() {
    // Save our form to cache.
    Botcha::setForm($this);
    // Save form to DB.
    BotchaFormModel::save($this);
  }

  public function delete() {
    // Save our form to cache.
    Botcha::unsetForm($this);
    // Save form to DB.
    BotchaFormModel::delete($this);
  }
}


/**
 * Dummy class, created for data consistency and for interface unifying.
 * When there is no form, this class is used as a handler.
 * It has no logic at all - by design.
 */
class BotchaFormNone extends BotchaForm {
  public function __construct($form_id = NULL) {
    $this->id = !empty($form_id) ? $form_id : 'none';
  }

  public function getRecipebook() {
    return new BotchaRecipebookNone();
  }

  public function save() {
    Botcha::unsetForm($this);
    // Save form to DB.
    BotchaFormModel::save($this);
  }
}
