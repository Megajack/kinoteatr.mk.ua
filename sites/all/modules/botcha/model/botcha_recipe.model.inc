<?php

/*
 * Model layer of the BotchaRecipe objects.
 */
class BotchaRecipeModel {

  protected static $recipes;

  public static function getRecipes($parameters = NULL) {
    $recipes = db_select('botcha_recipe', 'br')
      ->fields('br');
    if (!empty($parameters['recipes'])) {
      foreach ((array)$parameters['recipes'] as $recipe_id) {
        $recipes->condition('id', $recipe_id, 'IN');
      }
    }
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      $result = $recipes->execute()->fetchAllAssoc('id');
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
        $result = array();
      }
    }
    return $result;
  }

  public static function getRecipe($id) {
    if (empty(self::$recipes[$id])) {
      self::$recipes = self::getRecipes();
    }
    return self::$recipes[$id];
  }

  function save($recipe) {
    // Catching of PDOException helps to avoid WSOD during update use case. The
    // reason is in that form_alter is called before performing an update.
    // @see http://drupal.org/node/1828710
    try {
      // Save recipe to DB.
      db_merge('botcha_recipe')
        ->fields(array('id', 'classname', 'title', 'description'))
        ->values(array(
          'id' => $this->id,
          'classname' => $this->classname,
          'title' => $this->title,
          'description' => $this->description,
        ))->execute();
    } catch (Exception $e) {
      if ($e instanceof PDOException) {
        watchdog_exception('BOTCHA', $e, 'Please perform an update via update.php or reinstall the BOTCHA module to fix the reason of this warning! %type: !message in %function (line %line of %file).', array(), WATCHDOG_WARNING);
      }
    }
  }
}
