<?php

/*
 * Model layer of the BOTCHA application.
 */
module_load_include('inc', 'botcha', 'model/botcha_form.model');
module_load_include('inc', 'botcha', 'model/botcha_recipe.model');
module_load_include('inc', 'botcha', 'model/botcha_recipebook.model');

class BotchaModel {

  //protected static $recipebooks_forms;
  //protected static $recipebooks_recipes;

  /**
   * Just an alias for getRecipebooksForms.
   * @param array $parameters
   * @return array
   */
  public static function getFormsRecipebooks($parameters = array()) {
    $parameters['mode'] = !empty($parameters['mode']) ? $parameters['mode'] : 'recipebook';
    return self::getRecipebooksForms($parameters);
  }

  public static function getRecipebooksForms($parameters = array()) {
    // Get the value from the cache.
    //self::$recipebooks_forms = &drupal_static('botcha_recipebooks_forms');
    //if (empty(self::$recipebooks_forms) || $parameters['reset']) {
    $fields = array();
    switch ($parameters['mode']) {
      case 'form':
        //return self::$recipebooks_forms->fetchAllAssoc('form_id');
        $fields[] = 'form_id';
        break;
      case 'recipebook':
      default:
        //return self::$recipebooks_forms->fetchAllAssoc('rbid');
        $fields[] = 'rbid';
        break;
    }
    $rbf = db_select('botcha_recipebook_form', 'brf')
      ->fields('brf', $fields);
    if (!empty($parameters['recipebooks'])) {
      $rbf->condition('rbid', (array)$parameters['recipebooks'], 'IN');
    }
    if (!empty($parameters['forms'])) {
      $rbf->condition('form_id', (array)$parameters['forms'], 'IN');
    }
      //self::$recipebooks_forms = $rbf;
    //}
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      $result = $rbf->execute()->fetchCol();
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
        $result = array();
      }
    }
    if (count($result) == 1) {
      $result = current($result);
    }
    return $result;
  }

  public static function getRecipesRecipebooks($parameters = NULL) {
    return self::getRecipebooksRecipes($parameters);
  }

  public static function getRecipebooksRecipes($parameters = NULL) {
    // Get the value from the cache.
    //self::$recipebooks_recipes = &drupal_static('botcha_recipebooks_recipes');
    //if (empty(self::$recipebooks_recipes) || $parameters['reset']) {
    $fields = array();
    switch ($parameters['mode']) {
      case 'recipe':
        $fields[] = 'recipe_id';
        break;
      case 'recipebook':
      default:
        $fields[] = 'rbid';
        break;
    }
    $rbr = db_select('botcha_recipebook_recipe', 'brr')
      ->fields('brr', $fields);
    if (!empty($parameters['recipebooks'])) {
      $rbr->condition('rbid', (array)$parameters['recipebooks'], 'IN');
    }
    if (!empty($parameters['recipes'])) {
      $rbr->condition('recipe_id', (array)$parameters['recipes'], 'IN');
    }
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      $result = $rbr->execute()->fetchCol();
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
        $result = array();
      }
    }
    return $result;
  }
}
