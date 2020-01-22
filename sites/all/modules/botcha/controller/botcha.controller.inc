<?php

/*
 * Controller layer of the BOTCHA application.
 */
// @todo Migrate all necessary functions not to have this including.
module_load_include('inc', 'botcha', 'botcha');

module_load_include('inc', 'botcha', 'model/botcha.model');
module_load_include('inc', 'botcha', 'controller/botcha_form.controller');
module_load_include('inc', 'botcha', 'controller/botcha_recipe.controller');
module_load_include('inc', 'botcha', 'controller/botcha_recipebook.controller');

/**
 * Singleton realization of botcha application.
 */
class Botcha {

  protected static $recipebooks = array();
  protected static $recipes = array();
  protected static $forms = array();

  // Protect from any way of creation: initialize, cloning and unserialize.
  private function __construct() { /* ... @return Singleton */ }
  private function __clone()     { /* ... @return Singleton */ }
  private function __wakeup()    { /* ... @return Singleton */ }

  /**
   * Get a list of available BOTCHA form objects.
   * @param boolean $reset
   * @return BotchaForm
   */
  public static function getForms($reset = FALSE) {
    // Get the value from the cache.
    if (empty(self::$forms) || $reset) {
      $fs = array_keys(BotchaFormModel::getForms());
      foreach ($fs as $form_id) {
        // Save form to our cache.
        self::$forms[$form_id] = Botcha::getForm($form_id);
      }
      //self::$recipebooks = &drupal_static('botcha_recipebooks', self::$recipebooks);
    }
    return self::$forms;
  }

  /**
   * Gets a form from cache. If it does not exists in cache - gets from
   * database. If it does not exists there also - returns NULL.
   * @param string $form_id
   * @return BotchaForm
   */
  public static function getForm($form_id, $create = TRUE) {
    // @todo This static cache is buggy. Remake it if we need it or delete else.
    //if (empty(self::$forms[$form_id])) {
      if ($form = BotchaForm::getForm($form_id, $create)) {
        // Set relationships for this concrete form.
        foreach ((array) BotchaModel::getFormsRecipebooks(array(
            'forms' => $form_id,
          )) as $rbid) {
            $form = $form->setRecipebook($rbid);
        }
      }
      self::$forms[$form_id] = $form;
    //}
    return self::$forms[$form_id];
  }

  /**
   * Sets form to the cache.
   * @param BotchaForm $form
   */
  public static function setForm($form) {
    self::$forms[$form->id] = $form;
  }

  public static function unsetForm($form) {
    unset(self::$forms[$form->id]);
  }

  /**
   * Get a list of all BOTCHA recipe book objects.
   * @param boolean $reset
   * @return BotchaRecipebook
   */
  public static function getRecipebooks($reset = FALSE) {
    // Get the value from the cache.
    // @todo Botcha getRecipebooks Find a better way to check whether we need to reset a list of all recipe books or not.
    //   The problem is in the case when we have one recipe book cached - and it
    //   makes unusable our cache since it does not have all recipe books.
    if (empty(self::$recipebooks) || $reset) {
      $rbs = array_keys(BotchaRecipebookModel::getRecipebooks());
      foreach ($rbs as $rbid) {
        // Save recipe book to our cache.
        self::$recipebooks[$rbid] = Botcha::getRecipebook($rbid);
      }
      //self::$recipebooks = &drupal_static('botcha_recipebooks', self::$recipebooks);
    }
    return self::$recipebooks;
  }

  /**
   * Gets a recipe book from cache. If it does not exists in cache - gets from
   * database. If it does not exists there also - returns new recipe book or NULL
   * depending on input parameter.
   * @param string $id
   *   Machine name of the recipe book to look for.
   * @param boolean $create
   *   Determines whether we should initialize new recipe book or not.
   * @return BotchaRecipebook
   */
  public static function getRecipebook($id = 'default', $create = TRUE) {
    //if (empty(self::$recipebooks[$id])) {
      // Process constructing of recipe book object.
      // Get main parameters of recipe book.
      $recipebook = BotchaRecipebook::getRecipebook($id, $create);
      if (!($recipebook instanceof BotchaRecipebookNone)) {
        // Get recipe book relationships.
        // Get recipe book - form relationships.
        foreach ((array) BotchaModel::getRecipebooksForms(array(
              'mode' => 'form',
              'recipebooks' => $id,
            )) as $form_id) {
          $recipebook->setForm($form_id);
        }
        // Get recipe book - recipe relationships.
        foreach (BotchaModel::getRecipebooksRecipes(array(
              'mode' => 'recipe',
              'recipebooks' => $id,
            )) as $recipe_id) {
          $recipebook->setRecipe($recipe_id);
        }
        self::$recipebooks[$id] = $recipebook;
      }
    //}
    return $recipebook;
  }

  /**
   * Sets recipe book to the cache.
   * @param BotchaRecipebook $recipebook
   */
  public static function setRecipebook($recipebook) {
    self::$recipebooks[$recipebook->id] = $recipebook;
  }

  public static function unsetRecipebook($recipebook) {
    unset(self::$recipebooks[$recipebook->id]);
  }

  /**
   * Get a list of all BOTCHA recipes objects.
   * @param boolean $reset
   * @return array
   */
  public static function getRecipes($reset = FALSE) {
    // Get the value from the cache.
    self::$recipes = &drupal_static('botcha_recipes');
    if (empty(self::$recipes) || $reset) {
      $recipes = array_keys(BotchaRecipeModel::getRecipes());
      foreach ($recipes as $recipe_id) {
        // Save recipe to our cache.
        self::$recipes[$recipe_id] = Botcha::getRecipe($recipe_id);
      }
      //self::$recipes = &drupal_static('botcha_recipes', self::$recipes);
    }
    return self::$recipes;
  }

  /**
   * Gets a recipe from cache. If it does not exists in cache - gets from
   * database. If it does not exists there also - returns NULL.
   * @param string $id
   *   Machine name of the recipe book to look for.
   * @return BotchaRecipe
   */
  public static function getRecipe($id) {
    if (empty(self::$recipes[$id])) {
      $recipe = BotchaRecipe::getRecipe($id);
      // Set relationships for this concrete recipe.
      foreach (BotchaModel::getRecipesRecipebooks(array(
            'mode' => 'recipebook',
            'recipes' => $id,
          )) as $rbid) {
        $recipe->setRecipebook($rbid);
      }
      self::$recipes[$id] = $recipe;
    }
    return self::$recipes[$id];
  }
}
