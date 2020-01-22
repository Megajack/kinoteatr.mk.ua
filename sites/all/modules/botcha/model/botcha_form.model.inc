<?php

/*
 * Model layer of the BotchaForm objects.
 */
class BotchaFormModel {

  protected static $forms;

  public static function getForms($parameters = array()) {
    $forms = db_select('botcha_form', 'bf')
      ->fields('bf');
    if (!empty($parameters['forms'])) {
      foreach ((array) $parameters['forms'] as $form_id) {
        $forms->condition('id', $form_id, 'IN');
      }
    }
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      $result = $forms->execute()->fetchAllAssoc('id');
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
        $result = array();
      }
    }
    return $result;
  }

  public static function getForm($id) {
    //if (empty(self::$forms[$id])) {
      self::$forms = self::getForms();
    //}
    return !empty(self::$forms[$id]) ? (self::$forms[$id]) : NULL;
  }

  /**
   * Save form to the database.
   * @param BotchaForm $form
   */
  public static function save($form) {
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      // Save form.
      $forms = db_select('botcha_form', 'bf')
        ->fields('bf')
        ->condition('id', $form->id)
        ->execute()
        ->fetchCol();
      if (!count($forms)) {
        db_insert('botcha_form')
          ->fields(array('id' => $form->id))
          ->execute();
      }
      $recipebook = $form->getRecipebook();
      // Save form-recipe book relationship.
      db_merge('botcha_recipebook_form')
        ->key(array('form_id' => $form->id))
        ->fields(array(
          'rbid' => $recipebook->id,
        ))->execute();
      self::$forms[$form->id] = $form->id;
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
      }
    }
  }

  public static function delete($form) {
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      // Delete all data related to this form.
      db_delete('botcha_recipebook_form')
        ->condition('form_id', $form->id)
        ->execute();
      db_delete('botcha_form')
        ->condition('id', $form->id)
        ->execute();
      unset(self::$forms[$form->id]);
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
      }
    }
  }
}
