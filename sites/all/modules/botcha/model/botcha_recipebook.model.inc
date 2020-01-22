<?php

/*
 * Model layer of the BotchaRecipebook objects.
 */
class BotchaRecipebookModel {

  protected static $recipebooks;

  public static function getRecipebooks($parameters = NULL) {
    $recipebooks = db_select('botcha_recipebook', 'brb')
      ->fields('brb');
    if (!empty($parameters['recipebooks'])) {
      foreach ((array)$parameters['recipebooks'] as $rbid) {
        $recipebooks->condition('id', $rbid, 'IN');
      }
    }
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      $result = $recipebooks->execute()->fetchAllAssoc('id');
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
        $result = array();
      }
    }
    return $result;
  }

  public static function getRecipebook($id) {
    //if (empty(self::$recipebooks[$id])) {
      self::$recipebooks = self::getRecipebooks();
    //}
    return !empty(self::$recipebooks[$id]) ? (self::$recipebooks[$id]) : NULL;
  }

  /**
   * Save recipe book to the database.
   * @param BotchaRecipebook $recipebook
   */
  public static function save($recipebook) {
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      db_delete('botcha_recipebook_recipe')
        ->condition('rbid', $recipebook->id)
        ->execute();
      db_delete('botcha_recipebook_form')
        ->condition('rbid', $recipebook->id)
        ->execute();
      if (!($recipebook instanceof BotchaRecipebookNone)) {
        db_merge('botcha_recipebook')
          ->key(array('id' => $recipebook->id))
          ->fields(array(
            'title' => $recipebook->title,
            'description' => $recipebook->description,
          ))->execute();
        // Save relationships between recipe book and recipes to DB.
        $recipes = $recipebook->getRecipes();
        $insert = db_insert('botcha_recipebook_recipe');
        foreach ($recipes as $recipe) {
          $query = $insert->fields(array('rbid', 'recipe_id'))
            ->values(array(
              'rbid' => $recipebook->id,
              'recipe_id' => $recipe->id,
            ))->execute();
        }
        // Save relationships between recipe book and forms to DB.
        $forms = $recipebook->getForms();
        foreach ($forms as $form) {
          $brf = db_select('botcha_recipebook_form', 'brf')
            ->fields('brf', array('rbid'))
            ->condition('form_id', $form->id)
            ->execute()
            ->fetchCol();
          if (count($brf)) {
            $query = db_update('botcha_recipebook_form')
              ->condition('form_id', $form->id)
              ->fields(array('rbid' => $recipebook->id));
          }
          else {
            $query = db_insert('botcha_recipebook_form')
              ->fields(array('form_id' => $form->id, 'rbid' => $recipebook->id));
          }
          $query->execute();
          // Strange but true: db_merge does not work here, fails with integrity constraint violation.
          /*
          $query->key(array('form_id', 'rbid'), array($form->id, $recipebook->id))
            ->execute();
           *
           */
        }
      }
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
      }
    }
  }

  /**
   * Delete recipe book from the database.
   * @param BotchaRecipebook $recipebook
   */
  public static function delete($recipebook) {
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      db_delete('botcha_recipebook_recipe')
        ->condition('rbid', $recipebook->id)
        ->execute();
      db_delete('botcha_recipebook_form')
        ->condition('rbid', $recipebook->id)
        ->execute();
      db_delete('botcha_recipebook')
        ->condition('id', $recipebook->id)
        ->execute();
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
      }
    }
  }
}
